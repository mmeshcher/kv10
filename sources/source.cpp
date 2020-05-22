// Copyright 2020 <mmeshcher>

#include <boost/asio.hpp>

#include "Random.hpp"
#include "DataBase.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/unordered_map.hpp>
#include "Globals.hpp"

void init()
{
    boost::log::register_simple_formatter_factory<
            boost::log::trivial::severity_level,
            char
    >("Severity");
    static const std::string format =
    "[%TimeStamp%][%ThreadID%][%Severity%]: %Message%";

    auto sinkFile = boost::log::add_file_log(
            boost::log::keywords::file_name = "logs/log_%N.log",
            boost::log::keywords::rotation_size = 128 * 1024 * 1024,
            boost::log::keywords::auto_flush = true,
            boost::log::keywords::format = format);
    sinkFile->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::trace);

    static const boost::unordered_map<std::string,
     boost::log::trivial::severity_level> CONSOLE_FILTER = {
            {"debug", boost::log::trivial::info},
            {"info", boost::log::trivial::info},
            {"warning", boost::log::trivial::warning},
            {"error", boost::log::trivial::error},
    };

    auto sinkConsole = boost::log::add_console_log(
            std::cout,
            boost::log::keywords::format = format);
    sinkConsole->set_filter(
            boost::log::trivial::severity >=
             CONSOLE_FILTER.at(Globals::logLevel));

    boost::log::add_common_attributes();
}

int main(int argc, char *argv[])
{
    if (auto returnCode = programArguments(argc, argv); returnCode != 0) {
        return returnCode;
    }

    init();

    BOOST_LOG_TRIVIAL(debug) << "Log setup complete";
    BOOST_LOG_TRIVIAL(info) << "Input: " << Globals::input
                            << "\nOutput: " << Globals::output
                            << "\nThreads: " << Globals::threadAmount
                            << "\nLogLevel: " << Globals::logLevel;

    if (Globals::writeOnly) {
        BOOST_LOG_TRIVIAL(info) << "Creating random db...";

        DataBase actions{Globals::input};
        actions.create();
        actions.randomFill();

        return 0;
    }

    removeDirectoryIfExists(Globals::output);
    copyDirectory(Globals::input, Globals::output);

    DataBase actions{Globals::output};

    boost::asio::thread_pool pool(Globals::threadAmount);

    auto descriptors = actions.getFamilyDescriptorList();
    auto handlers = actions.open(descriptors);

    std::list<boost::unordered_map<std::string, std::string>> cachedRows;
    for (auto &family:handlers) {
        cachedRows.push_back(actions.getRows(family.get()));
        auto &rows = cachedRows.back();
        auto beginIterator = rows.cbegin();
        if (beginIterator != rows.cend()) {
        boost::asio::post(pool, [&actions, &family, beginIterator, &rows]() {
                actions.hashRows(family.get(), beginIterator, rows.cend());
            });
        }
    }
    pool.join();
}
