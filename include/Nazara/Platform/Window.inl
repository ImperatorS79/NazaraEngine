// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Nazara Engine - Platform module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Platform/Window.hpp>
#include <Nazara/Core/ErrorFlags.hpp>
#include <Nazara/Core/LockGuard.hpp>
#include <Nazara/Platform/Debug.hpp>

namespace Nz
{
	/*!
	* \class Nz::Window
	*/

	inline Window::Window(VideoMode mode, const String& title, WindowStyleFlags style) :
	Window()
	{
		ErrorFlags flags(ErrorFlag_ThrowException, true);
		Create(mode, title, style);
	}

	inline Window::Window(void* handle) :
	Window()
	{
		ErrorFlags flags(ErrorFlag_ThrowException, true);
		Create(handle);
	}

	inline void Window::Close()
	{
		m_closed = true; // The window will be closed at the next non-const IsOpen() call
	}

	inline void Window::EnableCloseOnQuit(bool closeOnQuit)
	{
		m_closeOnQuit = closeOnQuit;
	}

	inline void Window::EnableEventPolling(bool enable)
	{
		m_eventPolling = enable;
		if (!m_eventPolling)
		{
			while (!m_events.empty())
				m_events.pop();
		}
	}

	inline const CursorRef& Window::GetCursor() const
	{
		return m_cursor;
	}

	inline CursorController& Nz::Window::GetCursorController()
	{
		return m_cursorController;
	}

	inline EventHandler& Nz::Window::GetEventHandler()
	{
		return m_eventHandler;
	}

	inline bool Window::IsOpen(bool checkClosed)
	{
		if (!m_impl)
			return false;

		if (checkClosed && m_closed)
		{
			Destroy();
			return false;
		}

		return true;
	}

	inline bool Window::IsOpen() const
	{
		return m_impl != nullptr;
	}

	inline bool Window::IsValid() const
	{
		return m_impl != nullptr;
	}

	inline void Window::SetCursor(SystemCursor systemCursor)
	{
		SetCursor(Cursor::Get(systemCursor));
	}

	inline void Window::PushEvent(const WindowEvent& event)
	{
		if (!m_asyncWindow)
			HandleEvent(event);
		else
		{
			{
				LockGuard eventLock(m_eventMutex);

				m_pendingEvents.push_back(event);
			}

			if (m_waitForEvent)
			{
				m_eventConditionMutex.Lock();
				m_eventCondition.Signal();
				m_eventConditionMutex.Unlock();
			}
		}
	}

	inline WindowImpl* Window::GetImpl()
	{
		return m_impl;
	}

	inline const WindowImpl* Window::GetImpl() const
	{
		return m_impl;
	}
}

#include <Nazara/Platform/DebugOff.hpp>
