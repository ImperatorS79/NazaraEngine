// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Nazara Development Kit"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <NDK/Widgets/AbstractTextAreaWidget.hpp>

namespace Ndk
{
	inline void AbstractTextAreaWidget::EnableMultiline(bool enable)
	{
		m_multiLineEnabled = enable;
	}

	inline void AbstractTextAreaWidget::EnableTabWriting(bool enable)
	{
		m_tabEnabled = enable;
	}

	inline void AbstractTextAreaWidget::Erase(std::size_t glyphPosition)
	{
		Erase(glyphPosition, glyphPosition + 1U);
	}

	inline void AbstractTextAreaWidget::EraseSelection()
	{
		if (!HasSelection())
			return;

		Erase(GetGlyphIndex(m_cursorPositionBegin), GetGlyphIndex(m_cursorPositionEnd));
	}

	inline const AbstractTextAreaWidget::CharacterFilter& AbstractTextAreaWidget::GetCharacterFilter() const
	{
		return m_characterFilter;
	}

	inline const Nz::Vector2ui& AbstractTextAreaWidget::GetCursorPosition() const
	{
		return m_cursorPositionBegin;
	}

	Nz::Vector2ui AbstractTextAreaWidget::GetCursorPosition(std::size_t glyphIndex) const
	{
		const Nz::AbstractTextDrawer& textDrawer = GetTextDrawer();

		glyphIndex = std::min(glyphIndex, GetTextDrawer().GetGlyphCount());

		std::size_t lineCount = textDrawer.GetLineCount();
		std::size_t line = 0U;
		for (std::size_t i = line + 1; i < lineCount; ++i)
		{
			if (textDrawer.GetLine(i).glyphIndex > glyphIndex)
				break;

			line = i;
		}

		const auto& lineInfo = textDrawer.GetLine(line);

		Nz::Vector2ui cursorPos;
		cursorPos.y = static_cast<unsigned int>(line);
		cursorPos.x = static_cast<unsigned int>(glyphIndex - lineInfo.glyphIndex);

		return cursorPos;
	}

	inline EchoMode AbstractTextAreaWidget::GetEchoMode() const
	{
		return m_echoMode;
	}

	inline std::size_t AbstractTextAreaWidget::GetGlyphIndex() const
	{
		return GetGlyphIndex(m_cursorPositionBegin);
	}

	inline std::size_t AbstractTextAreaWidget::GetGlyphIndex(const Nz::Vector2ui& cursorPosition) const
	{
		const Nz::AbstractTextDrawer& textDrawer = GetTextDrawer();

		std::size_t glyphIndex = textDrawer.GetLine(cursorPosition.y).glyphIndex + cursorPosition.x;
		if (textDrawer.GetLineCount() > cursorPosition.y + 1)
			glyphIndex = std::min(glyphIndex, textDrawer.GetLine(cursorPosition.y + 1).glyphIndex - 1);
		else
			glyphIndex = std::min(glyphIndex, textDrawer.GetGlyphCount());

		return glyphIndex;
	}

	inline bool AbstractTextAreaWidget::HasSelection() const
	{
		return m_cursorPositionBegin != m_cursorPositionEnd;
	}

	inline bool AbstractTextAreaWidget::IsLineWrapEnabled() const
	{
		return m_isLineWrapEnabled;
	}

	inline bool AbstractTextAreaWidget::IsMultilineEnabled() const
	{
		return m_multiLineEnabled;
	}

	inline bool AbstractTextAreaWidget::IsTabWritingEnabled() const
	{
		return m_tabEnabled;
	}

	inline bool AbstractTextAreaWidget::IsReadOnly() const
	{
		return m_readOnly;
	}

	inline void AbstractTextAreaWidget::MoveCursor(int offset)
	{
		std::size_t cursorGlyph = GetGlyphIndex(m_cursorPositionBegin);
		if (offset >= 0)
			SetCursorPosition(cursorGlyph + static_cast<std::size_t>(offset));
		else
		{
			std::size_t nOffset = static_cast<std::size_t>(-offset);
			if (nOffset >= cursorGlyph)
				SetCursorPosition(0);
			else
				SetCursorPosition(cursorGlyph - nOffset);
		}
	}

	inline void AbstractTextAreaWidget::MoveCursor(const Nz::Vector2i& offset)
	{
		auto ClampOffset = [] (unsigned int cursorPosition, int cursorOffset) -> unsigned int
		{
			if (cursorOffset >= 0)
				return cursorPosition + cursorOffset;
			else
			{
				unsigned int nOffset = static_cast<unsigned int>(-cursorOffset);
				if (nOffset >= cursorPosition)
					return 0;
				else
					return cursorPosition - nOffset;
			}
		};

		Nz::Vector2ui cursorPosition = m_cursorPositionBegin;
		cursorPosition.x = ClampOffset(static_cast<unsigned int>(cursorPosition.x), offset.x);
		cursorPosition.y = ClampOffset(static_cast<unsigned int>(cursorPosition.y), offset.y);

		SetCursorPosition(cursorPosition);
	}

	inline Nz::Vector2ui AbstractTextAreaWidget::NormalizeCursorPosition(Nz::Vector2ui cursorPosition) const
	{
		const Nz::AbstractTextDrawer& textDrawer = GetTextDrawer();

		std::size_t lineCount = textDrawer.GetLineCount();
		if (cursorPosition.y >= lineCount)
			cursorPosition.y = static_cast<unsigned int>(lineCount - 1);

		const auto& lineInfo = textDrawer.GetLine(cursorPosition.y);
		if (cursorPosition.y + 1 < lineCount)
		{
			const auto& nextLineInfo = textDrawer.GetLine(cursorPosition.y + 1);
			cursorPosition.x = std::min(cursorPosition.x, static_cast<unsigned int>(nextLineInfo.glyphIndex - lineInfo.glyphIndex - 1));
		}

		return cursorPosition;
	}

	inline void AbstractTextAreaWidget::SetCharacterFilter(CharacterFilter filter)
	{
		m_characterFilter = std::move(filter);
	}

	inline void AbstractTextAreaWidget::SetCursorPosition(std::size_t glyphIndex)
	{
		Nz::Vector2ui position = GetCursorPosition(glyphIndex);
		Nz::Vector2ui newPosition = position;

		OnTextAreaCursorMove(this, &newPosition);

		if (position == newPosition)
			SetCursorPositionInternal(position);
		else
			SetCursorPositionInternal(GetGlyphIndex(newPosition));
	}

	inline void AbstractTextAreaWidget::SetCursorPosition(Nz::Vector2ui cursorPosition)
	{
		OnTextAreaCursorMove(this, &cursorPosition);

		return SetCursorPositionInternal(NormalizeCursorPosition(cursorPosition));
	}

	inline void AbstractTextAreaWidget::SetEchoMode(EchoMode echoMode)
	{
		m_echoMode = echoMode;

		UpdateDisplayText();
	}

	inline void AbstractTextAreaWidget::SetReadOnly(bool readOnly)
	{
		m_readOnly = readOnly;
		m_cursorEntity->Enable(!m_readOnly && HasFocus());
	}

	inline void AbstractTextAreaWidget::SetSelection(Nz::Vector2ui fromPosition, Nz::Vector2ui toPosition)
	{
		// Ensure begin is before end
		if (toPosition.y < fromPosition.y || (toPosition.y == fromPosition.y && toPosition.x < fromPosition.x))
			std::swap(fromPosition, toPosition);

		if (m_cursorPositionBegin != fromPosition || m_cursorPositionEnd != toPosition)
		{
			OnTextAreaSelection(this, &fromPosition, &toPosition);

			// Ensure begin is before end a second time (in case signal changed it)
			if (toPosition.y < fromPosition.y || (toPosition.y == fromPosition.y && toPosition.x < fromPosition.x))
				std::swap(fromPosition, toPosition);

			m_cursorPositionBegin = NormalizeCursorPosition(fromPosition);
			m_cursorPositionEnd = NormalizeCursorPosition(toPosition);

			RefreshCursor();
		}
	}

	inline void AbstractTextAreaWidget::Write(const Nz::String& text)
	{
		Write(text, GetGlyphIndex(m_cursorPositionBegin));
	}

	inline void AbstractTextAreaWidget::Write(const Nz::String& text, const Nz::Vector2ui& glyphPosition)
	{
		Write(text, GetGlyphIndex(glyphPosition));
	}

	void AbstractTextAreaWidget::SetCursorPositionInternal(std::size_t glyphIndex)
	{
		return SetCursorPositionInternal(GetCursorPosition(glyphIndex));
	}

	inline void AbstractTextAreaWidget::SetCursorPositionInternal(Nz::Vector2ui cursorPosition)
	{
		m_cursorPositionBegin = cursorPosition;
		m_cursorPositionEnd = m_cursorPositionBegin;

		RefreshCursor();
	}
}
