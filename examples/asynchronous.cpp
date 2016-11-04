#include <curl-asio.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/ptr_container/ptr_set.hpp>
#include <iostream>
#include <fstream>
#include <experimental/string_view>
#include <memory> // for std::auto_ptr

boost::ptr_set<curl::easy> active_downloads;

size_t completed = 0;

void handle_download_completed(const boost::system::error_code& err, std::string url, curl::easy* easy)
{
	++completed;
/*	if (!err)
	{
		std::cout << "Download of " << url << " completed" << std::endl;
	}
	else
	{
		std::cout << "Download of " << url << " failed: " << err.message() << std::endl;
	}*/
	
	active_downloads.erase(*easy);
}

size_t null_write_function (char* ptr, size_t size, size_t nmemb, void* userdata)
{
	const size_t actual_size = size * nmemb;
	return actual_size;
}

int debug_callback (curl::native::CURL*, curl::native::curl_infotype type, char* str, size_t strSize, void*)
{
	if (type == curl::native::CURLINFO_TEXT || type == curl::native::CURLINFO_HEADER_IN || type == curl::native::CURLINFO_HEADER_OUT)
		std::cout << std::experimental::string_view (str, strSize);

	return 0;
}

void start_download(curl::multi& multi, const std::string& url)
{
	std::auto_ptr<curl::easy> easy(new curl::easy(multi));
	
	// see 'Use server-provided file names' example for a more detailed implementation of this function which receives
	// the target file name from the server using libcurl's header callbacks
	std::string file_name = url.substr(url.find_last_of('/') + 1);
	
	easy->set_url(url);

	//easy->set_verbose (true);
	//easy->set_debug_callback (debug_callback);

	//easy->set_max_connects (3);
	
	//easy->set_sink(boost::make_shared<std::ofstream>(("./out/" + file_name).c_str(), std::ios::binary));
	easy->set_write_function (&null_write_function);
	easy->async_perform(boost::bind(handle_download_completed, boost::asio::placeholders::error, url, easy.get()));
	
	active_downloads.insert(easy);
}

int main (int argc, char* argv[])
try
{
	// this example program downloads all urls in the text file argv[1] to the current directory
	char* url_file_name = argv[1];
	size_t counter = 0;

	{	
		std::cerr.sync_with_stdio (false);
		std::cout.sync_with_stdio (false);

		// start by creating an io_service object
		boost::asio::io_service io_service;
		
		// construct an instance of curl::multi
		curl::multi manager(io_service);
		//manager.use_pipelining (true);
		manager.set_max_pipeline_legth (50);
		manager.set_max_connections (10);

		// treat each line in url_file_name as url and start a download from it
		std::ifstream url_file(url_file_name);
		
		while (!url_file.eof())
		{
			std::string url;
			std::getline(url_file, url);
			start_download(manager, url);
			if (++counter > 4800000000000000)
				break;//*/
		}

		// let Boost.Asio do its magic
		const auto handlersProcessed = io_service.run();
		std::cout << "Asio handlers: " << handlersProcessed << std::endl; 	
	}

	std::cout << "Started: " << counter << ", ended: " << completed << std::endl;
	
	return 0;
}
catch (const std::exception& err)
{
	std::cerr << err.what();
	return 1;
}
catch (...)
{
	std::cerr << "Err";
	return 1;
}