#include "battle_game/core/bullets/bounce_ball.h"

#include "battle_game/core/game_core.h"
#include "battle_game/core/particles/particles.h"

namespace battle_game::bullet {
BounceBall::BounceBall(GameCore *core,
                       uint32_t id,
                       uint32_t unit_id,
                       uint32_t player_id,
                       glm::vec2 position,
                       float rotation,
                       float damage_scale,
                       glm::vec2 velocity)
    : Bullet(core, id, unit_id, player_id, position, rotation, damage_scale),
      bounce_counts_(5),
      velocity_(velocity) {
}

void BounceBall::Render() {
  SetTransformation(position_, rotation_, glm::vec2{0.1f});
  SetColor(game_core_->GetPlayerColor(player_id_));
  SetTexture(BATTLE_GAME_ASSETS_DIR "textures/particle3.png");
  DrawModel(0);
}

void BounceBall::Update() {
  position_ += velocity_ * kSecondPerTick;
  bool should_die = false;
  if ((position_.x < -9.5f || position_.x > 9.5f) && bounce_counts_) {
    velocity_.x *= -1;
    bounce_counts_--;
  } else if ((position_.y < -9.5f || position_.y > 9.5f) && bounce_counts_) {
    velocity_.y *= -1;
    bounce_counts_--;
  }
  if (game_core_->IsBlockedByObstacles(position_)) {
    if (bounce_counts_ == 0)
      should_die = true;
    else {
      bounce_counts_--;
      if (game_core_->IsOutOfRange(position_)) {
        if (position_.x < -10.0f || position_.x > 10.0f) {
          velocity_.x *= -1;
        }
        else {
          velocity_.y *= -1;
        }
      } else {
        velocity_ *= -1;
      }
    }
  }

  auto &units = game_core_->GetUnits();
  for (auto &unit : units) {
    if (unit.first == unit_id_) {
      continue;
    }
    if (unit.second->IsHit(position_)) {
      game_core_->PushEventDealDamage(unit.first, id_, damage_scale_ * 10.0f);
      should_die = true;
    }
  }

  if (should_die) {
    if (bounce_counts_) {
      velocity_ *= -1;
      bounce_counts_--;
    } else {
      game_core_->PushEventRemoveBullet(id_);
    }
  }
}

BounceBall::~BounceBall() {
  for (int i = 0; i < 5; i++) {
    game_core_->PushEventGenerateParticle<particle::Smoke>(
        position_, rotation_, game_core_->RandomInCircle() * 2.0f, 0.2f,
        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}, 3.0f);
  }
}
}  // namespace battle_game::bullet
