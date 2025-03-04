#include "mod/MyMod.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/mod/RegisterHelper.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/NetworkIdentifier.h"
#include "mc/network/PacketObserver.h"
#include "mc/network/packet/DimensionDataPacket.h"
#include "mc/network/packet/Packet.h"
#include "mc/server/PropertiesSettings.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/DimensionDefinitionGroup.h"
#include "mc/world/level/dimension/DimensionHeightRange.h"

namespace my_mod {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() { return true; }

bool MyMod::enable() {
    auto& logger = getSelf().getLogger();
    logger.info("HigherWorld loaded!");
    logger.info("Overworld Max Height is now 512!");
    logger.info("Author: KobeBryant114514(Caixukun1919810)");
    return true;
}

bool MyMod::disable() { return true; }

} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::MyMod::getInstance());

LL_AUTO_TYPE_INSTANCE_HOOK(
    DimensionConstructor,
    ll::memory::HookPriority::Normal,
    Dimension,
    &Dimension::$ctor,
    void*,
    ILevel&              level,
    DimensionType        dimId,
    DimensionHeightRange heightRange,
    Scheduler&           callbackContext,
    std::string          name
) {
    if (dimId.id == 0) {
        heightRange.mMax = 512;
    }
    return origin(level, dimId, heightRange, callbackContext, name);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    PacketSend,
    HookPriority::Normal,
    PacketObserver,
    &PacketObserver::$packetSentTo,
    void,
    const NetworkIdentifier& netId,
    const Packet&            packet,
    uint                     size
) {
    if (packet.getId() == MinecraftPacketIds::StartGame) {
        auto defPkt = std::static_pointer_cast<DimensionDataPacket>(
            MinecraftPackets::createPacket(MinecraftPacketIds::DimensionDataPacket)
        );
        defPkt->mDimensionDefinitionGroup->mDimensionDefinitions.get()["minecraft:overworld"] =
            DimensionDefinitionGroup::DimensionDefinition(-64, 512, GeneratorType::Void);
        defPkt->sendToClient(netId, SubClientId::PrimaryClient);
    }
    return origin(netId, packet, size);
};

LL_AUTO_TYPE_INSTANCE_HOOK(
    ClientGen,
    ll::memory::HookPriority::Normal,
    PropertiesSettings,
    &PropertiesSettings::$ctor,
    void*,
    const std::string& filename
) {
    auto result                          = (PropertiesSettings*)origin(filename);
    result->mClientSideGenerationEnabled = false;
    return result;
}