#include "engine/WorldState.h"

WorldState::WorldState() {}

void WorldState::reset() {
    m_tick = 0;
    m_gameState = GameState::Ready;
    m_players.clear();
    m_ghosts.clear();
}

void WorldState::addPlayer(const Player &player) {
    m_players.insert(player.id(), player);
}

Player *WorldState::player(uint32_t id) {
    auto it = m_players.find(id);
    if (it != m_players.end()) {
        return &it.value();
    }
    return nullptr;
}

const Player *WorldState::player(uint32_t id) const {
    auto it = m_players.find(id);
    if (it != m_players.end()) {
        return &it.value();
    }
    return nullptr;
}
