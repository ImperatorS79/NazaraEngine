// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <memory>
#include <Nazara/Utility/Debug.hpp>

///TODO: Listener de font (cas où l'atlas a changé)

NzSimpleTextDrawer::NzSimpleTextDrawer() :
m_color(NzColor::White),
m_style(nzTextStyle_Regular)
{
	// SetFont(NzFont::GetDefault());
}

NzSimpleTextDrawer::~NzSimpleTextDrawer()
{
	if (m_font)
		m_font->RemoveResourceListener(this);
}

const NzRectui& NzSimpleTextDrawer::GetBounds() const
{
	if (!m_glyphUpdated)
		UpdateGlyphs();

	return m_bounds;
}

unsigned int NzSimpleTextDrawer::GetCharacterSize() const
{
	return m_characterSize;
}

const NzColor& NzSimpleTextDrawer::GetColor() const
{
	return m_color;
}

NzFont* NzSimpleTextDrawer::GetFont() const
{
	return m_font;
}

nzUInt32 NzSimpleTextDrawer::GetStyle() const
{
	return m_style;
}

void NzSimpleTextDrawer::SetCharacterSize(unsigned int characterSize)
{
	m_characterSize = characterSize;

	m_glyphUpdated = false;
}

void NzSimpleTextDrawer::SetColor(const NzColor& color)
{
	m_color = color;

	m_glyphUpdated = false;
}

void NzSimpleTextDrawer::SetFont(NzFont* font)
{
	if (m_font)
		m_font->RemoveResourceListener(this);

	m_font = font;
	if (m_font)
		m_font->AddResourceListener(this);

	m_glyphUpdated = false;
}

void NzSimpleTextDrawer::SetStyle(nzUInt32 style)
{
	m_style = style;

	m_glyphUpdated = false;
}

void NzSimpleTextDrawer::SetText(const NzString& str)
{
	m_text = str;

	m_glyphUpdated = false;
}

NzSimpleTextDrawer NzSimpleTextDrawer::Draw(const NzString& str, unsigned int characterSize, nzUInt32 style, const NzColor& color)
{
	///FIXME: Sans default font ça n'a aucun intérêt
	NzSimpleTextDrawer drawer;
	drawer.SetCharacterSize(characterSize);
	drawer.SetColor(color);
	drawer.SetStyle(style);
	drawer.SetText(str);

	return drawer;
}

NzSimpleTextDrawer NzSimpleTextDrawer::Draw(NzFont* font, const NzString& str, unsigned int characterSize, nzUInt32 style, const NzColor& color)
{
	NzSimpleTextDrawer drawer;
	drawer.SetCharacterSize(characterSize);
	drawer.SetColor(color);
	drawer.SetFont(font);
	drawer.SetStyle(style);
	drawer.SetText(str);

	return drawer;
}

NzFont* NzSimpleTextDrawer::GetFont(unsigned int index) const
{
	#if NAZARA_UTILITY_SAFE
	if (index > 0)
	{
		NazaraError("Font index out of range (" + NzString::Number(index) + " >= 1)");
		return nullptr;
	}
	#endif

	return m_font;
}

unsigned int NzSimpleTextDrawer::GetFontCount() const
{
	return 1;
}

const NzAbstractTextDrawer::Glyph& NzSimpleTextDrawer::GetGlyph(unsigned int index) const
{
	if (!m_glyphUpdated)
		UpdateGlyphs();

	return m_glyphs[index];
}

unsigned int NzSimpleTextDrawer::GetGlyphCount() const
{
	if (!m_glyphUpdated)
		UpdateGlyphs();

	return m_glyphs.size();
}

bool NzSimpleTextDrawer::OnResourceModified(const NzResource* resource, int index, unsigned int code)
{
	NazaraUnused(resource);
	NazaraUnused(index);

	#ifdef NAZARA_DEBUG
	if (m_font != resource)
	{
		NazaraInternalError("Not listening to " + NzString::Pointer(resource));
		return false;
	}
	#endif

	if (code == NzFont::ModificationCode_AtlasChanged ||
	    code == NzFont::ModificationCode_AtlasLayerChanged ||
	    code == NzFont::ModificationCode_GlyphCacheCleared)
	{
		m_glyphUpdated = false;
	}

	return true;
}

void NzSimpleTextDrawer::OnResourceReleased(const NzResource* resource, int index)
{
	NazaraUnused(resource);
	NazaraUnused(index);

	#ifdef NAZARA_DEBUG
	if (m_font != resource)
	{
		NazaraInternalError("Not listening to " + NzString::Pointer(resource));
		return;
	}
	#endif

	SetFont(nullptr);
}

void NzSimpleTextDrawer::UpdateGlyphs() const
{
	m_bounds.MakeZero();
	m_glyphs.clear();
	m_glyphUpdated = true;

	#if NAZARA_UTILITY_SAFE
	if (!m_font || !m_font->IsValid())
	{
		NazaraError("Invalid font");
		return;
	}
	#endif

	if (m_text.IsEmpty())
		return;

	///TODO: Itération UTF-8 => UTF-32 sans allocation de buffer (Exposer utf8cpp ?)
	unsigned int size;
	std::unique_ptr<char32_t[]> characters(m_text.GetUtf32Buffer(&size));
	if (!characters)
	{
		NazaraError("Invalid character set");
		return;
	}

	const NzFont::SizeInfo& sizeInfo = m_font->GetSizeInfo(m_characterSize);

	// "Curseur" de dessin
	NzVector2ui drawPos(0, m_characterSize);
	NzVector2ui lastPos(0, 0);

	m_glyphs.reserve(size);
	nzUInt32 previousCharacter = 0;
	for (unsigned int i = 0; i < size; ++i)
	{
		char32_t character = characters[i];

		if (previousCharacter != 0)
			drawPos.x += m_font->GetKerning(m_characterSize, previousCharacter, character);

		previousCharacter = character;

		bool whitespace = true;
		switch (character)
		{
			case ' ':
				drawPos.x += sizeInfo.spaceAdvance;
				break;

			case '\n':
				drawPos.x = 0;
				drawPos.y += sizeInfo.lineHeight;
				break;

			case '\t':
				drawPos.x += sizeInfo.spaceAdvance*4;
				break;

			default:
				whitespace = false;
				break;
		}

		if (whitespace)
			continue; // Inutile d'avoir un glyphe pour un espace blanc

		const NzFont::Glyph& fontGlyph = m_font->GetGlyph(m_characterSize, m_style, character);
		if (!fontGlyph.valid)
			continue; // Le glyphe n'a pas été correctement chargé, que pouvons-nous faire d'autre que le passer

		Glyph glyph;
		glyph.atlas = m_font->GetAtlas()->GetLayer(fontGlyph.layerIndex);
		glyph.atlasRect = fontGlyph.atlasRect;
		glyph.color = m_color;
		glyph.flipped = fontGlyph.flipped;

		float advance = fontGlyph.advance;

		NzRectf bounds(fontGlyph.aabb);
		bounds.x += drawPos.x;
		bounds.y += drawPos.y;

		if (fontGlyph.requireFauxBold)
		{
			// On va agrandir le glyphe pour simuler le gras (idée moisie, mais idée quand même)
			NzVector2f center = bounds.GetCenter();

			bounds.width *= 1.1f;
			bounds.height *= 1.1f;

			// On le replace à la bonne hauteur
			NzVector2f offset(bounds.GetCenter() - center);
			bounds.y -= offset.y;

			// On ajuste l'espacement
			advance *= 1.1f;
		}

		// On "penche" le glyphe pour obtenir un semblant d'italique
		float italic = (fontGlyph.requireFauxItalic) ? 0.208f : 0.f;
		float italicTop = italic * bounds.y;
		float italicBottom = italic * bounds.GetMaximum().y;

		glyph.corners[0].Set(bounds.x - italicTop, bounds.y);
		glyph.corners[1].Set(bounds.x + bounds.width - italicTop, bounds.y);
		glyph.corners[2].Set(bounds.x - italicBottom, bounds.y + bounds.height);
		glyph.corners[3].Set(bounds.x + bounds.width - italicBottom, bounds.y + bounds.height);

		m_glyphs.push_back(glyph);

		lastPos = drawPos;
		drawPos.x += advance;
	}

	m_bounds.ExtendTo(lastPos);
}
