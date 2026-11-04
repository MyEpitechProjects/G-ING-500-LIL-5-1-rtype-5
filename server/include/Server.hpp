#ifndef SERVER_HPP
    #define SERVER_HPP

    #include "NetworkHandler.hpp"
    #include "Session.hpp"
    #include "WakeSignal.hpp"
    #include "Instance.hpp"
    #include "ThreadPool.hpp"
    #include <string>
    #include <thread>
    #include <queue>
    #include <memory>
    #include <ranges>
    #include <string>
    #include <tuple>
    #include <utility>
    #include <vector>

namespace rtype {
    class Server {
        private:
            bool _running = true;
            std::shared_ptr<WakeSignal> _wakeSignal;
            std::chrono::_V2::steady_clock::duration _tickPeriod;

            Net::NetworkHandler _netHandler;
            std::thread _netThread;

            std::mutex _instancesMutex;
            std::unordered_map<std::uint32_t, Instance> _instances;
            PoolThread _pool;

            std::unordered_map<asio::ip::udp::endpoint, Session> _sessions;

            std::size_t _idCounterSession;
            std::size_t _idEventCounter;
            std::size_t _idInstanceCounter;

            //--------------------------------PACKET HANDLING-----------------------------------//

            void handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet>& incomingPacket);
            void handlePacket(Net::HelloPacket &packet, Net::udp::endpoint sender);
            void handlePacket(Net::CreateInstancePacket &packet, Net::udp::endpoint sender);
            void handlePacket(Net::InstancePacket &packet, Net::udp::endpoint sender);
            void handlePacket(Net::ListInstancesPacket &packet, Net::udp::endpoint sender);
            void handlePacket(Net::InputPacket &packet, Net::udp::endpoint sender);
            void handlePacket(Net::EventPacket &packet, Net::udp::endpoint sender);
            void handleChat(const Net::ChatEvent &chat, Net::udp::endpoint sender, std::uint32_t sessionId);

            Session &createSession(asio::ip::udp::endpoint endpoint);
            std::uint32_t createInstance();
            bool joinInstance(Session &session, std::uint32_t instanceId);
            void leaveInstance(Session &session);

            Net::SnapshotPacket convertGameOutput(const GameOutput &snapshot) const;

            void broadcast(Net::PacketCodec::Packet packet);

            void updateInstances();
            void dispatchOutputs();
            void dispatchOutput(std::uint32_t instanceId, const GameOutput &output);
            void sendToInstance(std::uint32_t instanceId, Net::PacketCodec::Packet packet);

            void removeInactiveClients();

            void insertPendingEvent(std::size_t client_id, PendingEvent event, std::uint32_t event_id);
            Net::EventPacket createEntityEvent(Net::EventType type, std::size_t entityId);
            Net::EventPacket createChatEvent(std::size_t senderId, const std::string &text);
            void broadcastEvent(std::uint32_t instanceId, Net::EventPacket outPacket);
            void erasePendingEvent(asio::ip::udp::endpoint sender, std::uint32_t event_id, std::size_t sessionId);

            void handlePendingEvents();
            
        public:
            Server(int port, std::size_t tickRate);
            ~Server();

            void run();
    };
}

#endif