#include <iostream>
#include <thread>
#include <chrono>

#include "Config.hpp"
#include "SyncClient.hpp"
#include <boost/property_tree/json_parser.hpp>
using namespace std;

asio::io_service io_svc;
volatile bool killswitch;
int main(int argc, char** argv)
{
    Config config("settings.ini");
    SyncClient client(config.getSyncServer(), config.getPort());
    killswitch = false;
    std::thread t([&](void)->void
    {
        while(!killswitch)
        {
            auto fonts = client.fetchTrackedFonts(io_svc);
            for(auto font : fonts)
            {
                std::cout << font.getName() << std::endl;
                std::cout << font.getCategory() << std::endl;
                std::cout << font.getType() << std::endl;
                std::cout << font.getRemotePath() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(config.getSyncInterval()));
        }
    });
    t.join();
    return 0;
}

