#pragma once

#include <memory>
#include "entity/EntityId.h"

namespace voxel {

class World;
typedef std::shared_ptr<World> WorldPtr;

}

namespace ai {

class Zone;
class Server;

}

namespace cooldown {

class CooldownProvider;
typedef std::shared_ptr<CooldownProvider> CooldownProviderPtr;

}

namespace attrib {

class ContainerProvider;
typedef std::shared_ptr<ContainerProvider> ContainerProviderPtr;

}

namespace network {

class MessageSender;
typedef std::shared_ptr<MessageSender> MessageSenderPtr;

}

namespace backend {

class AIRegistry;
typedef std::shared_ptr<AIRegistry> AIRegistryPtr;

class AILoader;
typedef std::shared_ptr<AILoader> AILoaderPtr;

class Entity;
typedef std::shared_ptr<Entity> EntityPtr;

class User;
typedef std::shared_ptr<User> UserPtr;

class Npc;
typedef std::shared_ptr<Npc> NpcPtr;

class EntityStorage;
typedef std::shared_ptr<EntityStorage> EntityStoragePtr;

class PoiProvider;
typedef std::shared_ptr<PoiProvider> PoiProviderPtr;

}

namespace core {

class TimeProvider;
typedef std::shared_ptr<TimeProvider> TimeProviderPtr;

}
