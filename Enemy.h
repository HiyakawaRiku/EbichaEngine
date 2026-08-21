#pragma once
#include "BaseCharacter.h"
#include "Physics3D.h"
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <queue>

class Enemy : public BaseCharacter
{
public:
	// 攻撃の種類（HatSphere吐き出し攻撃を追加）
	enum class AttackType {
		Charge,           // 1. 通常突進
		BouncingCharge,   // 2. バウンド突進
		GigaSlam,         // 3. 大技：跳躍落下叩きつけ
		SpawnHatSphere    // 4. HatSphere 吐き出し攻撃
	};

	// 敵の状態定義
	enum class State {
		Normal,     // 浮遊・移動
		AttackPrep, // 攻撃予備動作（溜め）
		AttackLock,
		Attacking,  // 攻撃実行中
		AttackCool  // 攻撃後の隙（クールダウン／ひるみ）
	};

	// HatSphere 生成リクエスト用構造体
	struct SpawnRequest {
		Vector3 position;
		Vector3 velocity;
	};

	// ★ 生成リクエスト用の構造体
	struct HatSphereSpawnRequest {
		Vector3 position;
		Vector3 velocity;
	};

	void Initialize();
	void Update(Camera* activeCamera) override;
	void Draw() override;

	bool IsAttacking() const { return state_ == State::Attacking; }
	bool IsGroggy() const { return state_ == State::AttackCool && currentAttackType_ == AttackType::GigaSlam; }

	void SetTargetPlayer(const BaseCharacter* player) { targetPlayer_ = player; }
	void SpawnHatSpheresRandomly(int count);

	// キューブ敵としての AABB 当たり判定を取得
	AABB GetAABB() const {
		Vector3 halfSize = { 1.2f, 1.2f, 1.2f };
		Vector3 pos = transformBase_.translate;
		return AABB{
			{ pos.x - halfSize.x, pos.y - halfSize.y, pos.z - halfSize.z },
			{ pos.x + halfSize.x, pos.y + halfSize.y, pos.z + halfSize.z }
		};
	}

	Vector3& GetHalfSize() { return halfSize_; }
	void SetHalfSize(const Vector3& halfSize) { halfSize_ = halfSize; }

	// GameScene 側で HatSphere の生成リクエストをチェック・取り出す
	bool PopHatSphereSpawnRequest(Vector3& outPos, Vector3& outVel) {
		if (spawnRequests_.empty()) return false;
		auto req = spawnRequests_.back();
		spawnRequests_.pop_back();
		outPos = req.position;
		outVel = req.velocity;
		return true;
	}

	// HP & ダメージ関連機能
	void TakeDamage(int damage);
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp_; }
	bool IsDead() const { return hp_ <= 0; }

private:
	void UpdateNormal();
	void UpdateAttack();
	void UpdateFloating();

private:
	State state_ = State::Normal;
	AttackType currentAttackType_ = AttackType::Charge;

	const BaseCharacter* targetPlayer_ = nullptr;

	// HP・被弾用パラメータ
	static inline const int kMaxHp_ = 25;
	int hp_ = kMaxHp_;
	bool isInvincible_ = false;
	float invincibleTimer_ = 0.0f;
	static inline const float kInvincibleTime = 10.0f;

	// 浮遊用パラメータ
	float floatTimer_ = 0.0f;
	static inline const float kBaseHeight = 2.5f;
	static inline const float kFloatSpeed = 0.05f;
	static inline const float kFloatAmplitude = 0.3f;

	// ターゲット位置・方向保持用
	Vector3 chargeDirection_{ 0.0f, 0.0f, 1.0f };
	Vector3 slamTargetPos_{ 0.0f, 0.0f, 0.0f };

	// タイマー関連
	float attackIntervalTimer_ = 0.0f;
	float attackStateTimer_ = 0.0f;

	static inline const float kAttackInterval = 140.0f;
	static inline const float kPrepDuration = 40.0f;
	static inline const float kLockDuration = 60.0f;
	static inline const float kAttackDuration = 20.0f;
	static inline const float kCoolDuration = 45.0f;

	static inline const float kAttackDashSpeed = 0.5f;

	// HatSphere 生成リクエストキュー
	std::vector<SpawnRequest> spawnRequests_;

	// 予告線・ターゲット表示用パラメータ
	bool isShowWarning_ = false;
	float warningRadius_ = 2.0f;

	Vector3 halfSize_ = { 1.2f, 1.2f, 1.2f };
};