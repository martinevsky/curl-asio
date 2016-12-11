/**
	curl-asio: wrapper for integrating libcurl with boost.asio applications
	Copyright (c) 2013 Oliver Kuckertz <oliver.kuckertz@mologie.de>
	See COPYING for license information.

	C++ wrapper for libcurl's share interface
*/

#include <asio.hpp>

#include <curl-asio/error_code.h>
#include <curl-asio/share.h>

using namespace curl;

share::share()
{
	initref_ = initialization::ensure_initialization();
	handle_ = native::curl_share_init();

	if (!handle_)
	{
		throw std::bad_alloc();
	}

	set_lock_function(&share::lock);
	set_unlock_function(&share::unlock);
	set_user_data(this);
}

share::~share()
{
	if (handle_)
	{
		native::curl_share_cleanup(handle_);
		handle_ = 0;
	}
}

void share::set_share_cookies(bool enabled)
{
	std::error_code ec(native::curl_share_setopt(handle_, enabled ? native::CURLSHOPT_SHARE : native::CURLSHOPT_UNSHARE, native::CURL_LOCK_DATA_COOKIE));
	asio::detail::throw_error(ec);
}

void share::set_share_dns(bool enabled)
{
	std::error_code ec(native::curl_share_setopt(handle_, enabled ? native::CURLSHOPT_SHARE : native::CURLSHOPT_UNSHARE, native::CURL_LOCK_DATA_DNS));
	asio::detail::throw_error(ec);
}

void share::set_share_ssl_session(bool enabled)
{
	std::error_code ec(native::curl_share_setopt(handle_, enabled ? native::CURLSHOPT_SHARE : native::CURLSHOPT_UNSHARE, native::CURL_LOCK_DATA_SSL_SESSION));
	asio::detail::throw_error(ec);
}

void share::set_lock_function(lock_function_t lock_function)
{
	std::error_code ec(native::curl_share_setopt(handle_, native::CURLSHOPT_LOCKFUNC, lock_function));
	asio::detail::throw_error(ec);
}

void share::set_unlock_function(unlock_function_t unlock_function)
{
	std::error_code ec(native::curl_share_setopt(handle_, native::CURLSHOPT_UNLOCKFUNC, unlock_function));
	asio::detail::throw_error(ec);
}

void share::set_user_data(void* user_data)
{
	std::error_code ec(native::curl_share_setopt(handle_, native::CURLSHOPT_USERDATA, user_data));
	asio::detail::throw_error(ec);
}

void share::lock(native::CURL* handle, native::curl_lock_data data, native::curl_lock_access access, void* userptr)
{
	share* self = static_cast<share*>(userptr);
	self->mutex_.lock();
}

void share::unlock(native::CURL* handle, native::curl_lock_data data, void* userptr)
{
	share* self = static_cast<share*>(userptr);
	self->mutex_.unlock();
}
