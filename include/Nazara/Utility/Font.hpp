// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

///TODO: FontManager ?

#pragma once

#ifndef NAZARA_FONT_HPP
#define NAZARA_FONT_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Core/ObjectLibrary.hpp>
#include <Nazara/Core/ObjectRef.hpp>
#include <Nazara/Core/Resource.hpp>
#include <Nazara/Core/ResourceLoader.hpp>
#include <Nazara/Core/ResourceParameters.hpp>
#include <Nazara/Utility/AbstractAtlas.hpp>
#include <Nazara/Utility/Enums.hpp>
#include <memory>
#include <unordered_map>

namespace Nz
{
	struct NAZARA_UTILITY_API FontParams : ResourceParameters
	{
		bool IsValid() const;
	};

	class Font;
	class FontData;

	struct FontGlyph;

	using FontConstRef = ObjectRef<const Font>;
	using FontLibrary = ObjectLibrary<Font>;
	using FontLoader = ResourceLoader<Font, FontParams>;
	using FontRef = ObjectRef<Font>;

	class NAZARA_UTILITY_API Font : public RefCounted, public Resource
	{
		friend FontLibrary;
		friend FontLoader;
		friend class Utility;

		public:
			struct Glyph;
			struct SizeInfo;

			Font();
			Font(const Font&) = delete;
			Font(Font&&) = delete;
			~Font();

			void ClearGlyphCache();
			void ClearKerningCache();
			void ClearSizeInfoCache();

			bool Create(FontData* data);
			void Destroy();

			bool ExtractGlyph(unsigned int characterSize, char32_t character, TextStyleFlags style, float outlineThickness, FontGlyph* glyph) const;

			const std::shared_ptr<AbstractAtlas>& GetAtlas() const;
			std::size_t GetCachedGlyphCount(unsigned int characterSize, TextStyleFlags style, float outlineThickness) const;
			std::size_t GetCachedGlyphCount() const;
			String GetFamilyName() const;
			int GetKerning(unsigned int characterSize, char32_t first, char32_t second) const;
			const Glyph& GetGlyph(unsigned int characterSize, TextStyleFlags style, float outlineThickness, char32_t character) const;
			unsigned int GetGlyphBorder() const;
			unsigned int GetMinimumStepSize() const;
			const SizeInfo& GetSizeInfo(unsigned int characterSize) const;
			String GetStyleName() const;

			bool IsValid() const;

			bool Precache(unsigned int characterSize, TextStyleFlags style, float outlineThickness, char32_t character) const;
			bool Precache(unsigned int characterSize, TextStyleFlags style, float outlineThickness, const String& characterSet) const;

			void SetAtlas(const std::shared_ptr<AbstractAtlas>& atlas);
			void SetGlyphBorder(unsigned int borderSize);
			void SetMinimumStepSize(unsigned int minimumStepSize);

			Font& operator=(const Font&) = delete;
			Font& operator=(Font&&) = delete;

			static std::shared_ptr<AbstractAtlas> GetDefaultAtlas();
			static const FontRef& GetDefault();
			static unsigned int GetDefaultGlyphBorder();
			static unsigned int GetDefaultMinimumStepSize();

			static FontRef OpenFromFile(const String& filePath, const FontParams& params = FontParams());
			static FontRef OpenFromMemory(const void* data, std::size_t size, const FontParams& params = FontParams());
			static FontRef OpenFromStream(Stream& stream, const FontParams& params = FontParams());

			template<typename... Args> static FontRef New(Args&&... args);

			static void SetDefaultAtlas(const std::shared_ptr<AbstractAtlas>& atlas);
			static void SetDefaultGlyphBorder(unsigned int borderSize);
			static void SetDefaultMinimumStepSize(unsigned int minimumStepSize);

			struct Glyph
			{
				Recti aabb;
				Rectui atlasRect;
				bool requireFauxBold;
				bool requireFauxItalic;
				bool flipped;
				bool valid;
				float fauxOutlineThickness;
				int advance;
				unsigned int layerIndex;
			};

			struct SizeInfo
			{
				int spaceAdvance;
				unsigned int lineHeight;
				float underlinePosition;
				float underlineThickness;
			};

			// Signals:
			NazaraSignal(OnFontAtlasChanged, const Font* /*font*/);
			NazaraSignal(OnFontAtlasLayerChanged, const Font* /*font*/, AbstractImage* /*oldLayer*/, AbstractImage* /*newLayer*/);
			NazaraSignal(OnFontDestroy, const Font* /*font*/);
			NazaraSignal(OnFontGlyphCacheCleared, const Font* /*font*/);
			NazaraSignal(OnFontKerningCacheCleared, const Font* /*font*/);
			NazaraSignal(OnFontRelease, const Font* /*font*/);
			NazaraSignal(OnFontSizeInfoCacheCleared, const Font* /*font*/);

		private:
			using GlyphMap = std::unordered_map<char32_t, Glyph>;

			UInt64 ComputeKey(unsigned int characterSize, TextStyleFlags style, float outlineThickness) const;
			void OnAtlasCleared(const AbstractAtlas* atlas);
			void OnAtlasLayerChange(const AbstractAtlas* atlas, AbstractImage* oldLayer, AbstractImage* newLayer);
			void OnAtlasRelease(const AbstractAtlas* atlas);
			const Glyph& PrecacheGlyph(GlyphMap& glyphMap, unsigned int characterSize, TextStyleFlags style, float outlineThickness, char32_t character) const;

			static bool Initialize();
			static void Uninitialize();

			NazaraSlot(AbstractAtlas, OnAtlasCleared, m_atlasClearedSlot);
			NazaraSlot(AbstractAtlas, OnAtlasLayerChange, m_atlasLayerChangeSlot);
			NazaraSlot(AbstractAtlas, OnAtlasRelease, m_atlasReleaseSlot);

			std::shared_ptr<AbstractAtlas> m_atlas;
			std::unique_ptr<FontData> m_data;
			mutable std::unordered_map<UInt64, std::unordered_map<UInt64, int>> m_kerningCache;
			mutable std::unordered_map<UInt64, GlyphMap> m_glyphes;
			mutable std::unordered_map<UInt64, SizeInfo> m_sizeInfoCache;
			unsigned int m_glyphBorder;
			unsigned int m_minimumStepSize;

			static std::shared_ptr<AbstractAtlas> s_defaultAtlas;
			static FontRef s_defaultFont;
			static FontLibrary::LibraryMap s_library;
			static FontLoader::LoaderList s_loaders;
			static unsigned int s_defaultGlyphBorder;
			static unsigned int s_defaultMinimumStepSize;
	};
}

#include <Nazara/Utility/Font.inl>

#endif // NAZARA_FONT_HPP
