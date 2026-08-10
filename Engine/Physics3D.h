#pragma once
#include "EMath.h"

// =================================================================
// 衝突プリミティブ定義
// =================================================================

using Point = Vector3;

struct AABB {
	Vector3 min;
	Vector3 max;

	void encapsulate(const Point& point) {
		min.x = (std::min)(min.x, point.x);
		min.y = (std::min)(min.y, point.y);
		min.z = (std::min)(min.z, point.z);
		max.x = (std::max)(max.x, point.x);
		max.y = (std::max)(max.y, point.y);
		max.z = (std::max)(max.z, point.z);
	}
};

struct BSphere {
	Point center;
	float radius{ 1.0f };
};

struct Ray {
	Point origin;
	Vector3 direction{ 0.0f, 0.0f, 1.0f };
};

struct Plane {
	Vector3 normal{ 0.0f, 1.0f, 0.0f };
	float distance{ 0.0f };
};

struct OBB {
	Point center;
	Vector3 extents;
	Matrix4x4 transform;

	Vector3 getAxis(int index) const {
		return transform.getAxis(index).normalized();
	}
};

struct HitInfo {
	bool hit{ false };
	float distance{ 0.0f };
	Point point;
	Vector3 normal;
};

namespace Physics3D {


	// =================================================================
	// 衝突判定クラス (Intersection Tests)
	// =================================================================

		// -------------------------------------------------------------
		// 点 (Point) 関連
		// -------------------------------------------------------------

		// 点 vs 点
	static bool IsCollision(const Point& a, const Point& b, float tolerance = 0.0001f) {
		return (a - b).lengthSq() <= (tolerance * tolerance);
	}

	// 点 vs 球
	static bool IsCollision(const Point& point, const BSphere& sphere) {
		return (point - sphere.center).lengthSq() <= (sphere.radius * sphere.radius);
	}
	static bool IsCollision(const BSphere& sphere, const Point& point) {
		return IsCollision(point, sphere);
	}

	// 点 vs AABB
	static bool IsCollision(const Point& point, const AABB& box) {
		return (point.x >= box.min.x && point.x <= box.max.x) &&
			(point.y >= box.min.y && point.y <= box.max.y) &&
			(point.z >= box.min.z && point.z <= box.max.z);
	}
	static bool IsCollision(const AABB& box, const Point& point) {
		return IsCollision(point, box);
	}

	// 点 vs OBB
	static bool IsCollision(const Point& point, const OBB& obb) {
		Vector3 d = point - obb.center;
		for (int i = 0; i < 3; ++i) {
			Vector3 axis = obb.getAxis(i);
			float dist = Vector3::dot(d, axis);
			float limit = (i == 0) ? obb.extents.x : (i == 1) ? obb.extents.y : obb.extents.z;

			if (dist > limit || dist < -limit) {
				return false;
			}
		}
		return true;
	}
	static bool IsCollision(const OBB& obb, const Point& point) {
		return IsCollision(point, obb);
	}

	// 点 vs 平面
	static bool IsCollision(const Point& point, const Plane& plane, float tolerance = 0.0001f) {
		float dist = Vector3::dot(plane.normal, point) + plane.distance;
		return std::abs(dist) <= tolerance;
	}
	static bool IsCollision(const Plane& plane, const Point& point, float tolerance = 0.0001f) {
		return IsCollision(point, plane, tolerance);
	}

	// 点 vs レイ
	static bool IsCollision(const Point& point, const Ray& ray, float tolerance = 0.001f) {
		Vector3 d = point - ray.origin;
		float t = Vector3::dot(d, ray.direction);
		if (t < 0.0f) return false;

		Point projection = ray.origin + ray.direction * t;
		return (point - projection).lengthSq() <= (tolerance * tolerance);
	}
	static bool IsCollision(const Ray& ray, const Point& point, float tolerance = 0.001f) {
		return IsCollision(point, ray, tolerance);
	}

	// -------------------------------------------------------------
	// 球 (BSphere) 関連
	// -------------------------------------------------------------

	// 球 vs 球
	static bool IsCollision(const BSphere& a, const BSphere& b) {
		float distanceSq = (a.center - b.center).lengthSq();
		float radiusSum = a.radius + b.radius;
		return distanceSq <= (radiusSum * radiusSum);
	}

	// 球 vs AABB
	static bool IsCollision(const BSphere& sphere, const AABB& box) {
		float closestX = std::clamp(sphere.center.x, box.min.x, box.max.x);
		float closestY = std::clamp(sphere.center.y, box.min.y, box.max.y);
		float closestZ = std::clamp(sphere.center.z, box.min.z, box.max.z);

		Point closestPoint{ closestX, closestY, closestZ };
		float distanceSq = (sphere.center - closestPoint).lengthSq();

		return distanceSq <= (sphere.radius * sphere.radius);
	}
	static bool IsCollision(const AABB& box, const BSphere& sphere) {
		return IsCollision(sphere, box);
	}

	// -------------------------------------------------------------
	// AABB 関連
	// -------------------------------------------------------------

	// AABB vs AABB
	static bool IsCollision(const AABB& a, const AABB& b) {
		if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
		if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
		if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
		return true;
	}

	// -------------------------------------------------------------
	// OBB 関連
	// -------------------------------------------------------------

	// OBB vs OBB (分離軸定理)
	static bool IsCollision(const OBB& a, const OBB& b) {
		float ra, rb;
		float R[3][3], AbsR[3][3];

		Vector3 A[3] = { a.getAxis(0), a.getAxis(1), a.getAxis(2) };
		Vector3 B[3] = { b.getAxis(0), b.getAxis(1), b.getAxis(2) };

		Vector3 v = b.center - a.center;
		Vector3 T = { Vector3::dot(v, A[0]), Vector3::dot(v, A[1]), Vector3::dot(v, A[2]) };

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				R[i][j] = Vector3::dot(A[i], B[j]);
				AbsR[i][j] = std::abs(R[i][j]) + 1e-4f;
			}
		}

		for (int i = 0; i < 3; i++) {
			ra = (i == 0) ? a.extents.x : (i == 1) ? a.extents.y : a.extents.z;
			rb = b.extents.x * AbsR[i][0] + b.extents.y * AbsR[i][1] + b.extents.z * AbsR[i][2];
			if (std::abs((i == 0) ? T.x : (i == 1) ? T.y : T.z) > ra + rb) return false;
		}

		for (int i = 0; i < 3; i++) {
			ra = a.extents.x * AbsR[0][i] + a.extents.y * AbsR[1][i] + a.extents.z * AbsR[2][i];
			rb = (i == 0) ? b.extents.x : (i == 1) ? b.extents.y : b.extents.z;
			float t = std::abs(T.x * R[0][i] + T.y * R[1][i] + T.z * R[2][i]);
			if (t > ra + rb) return false;
		}

		ra = a.extents.y * AbsR[2][0] + a.extents.z * AbsR[1][0];
		rb = b.extents.y * AbsR[0][2] + b.extents.z * AbsR[0][1];
		if (std::abs(T.z * R[1][0] - T.y * R[2][0]) > ra + rb) return false;

		ra = a.extents.y * AbsR[2][1] + a.extents.z * AbsR[1][1];
		rb = b.extents.x * AbsR[0][2] + b.extents.z * AbsR[0][0];
		if (std::abs(T.z * R[1][1] - T.y * R[2][1]) > ra + rb) return false;

		ra = a.extents.y * AbsR[2][2] + a.extents.z * AbsR[1][2];
		rb = b.extents.x * AbsR[0][1] + b.extents.y * AbsR[0][0];
		if (std::abs(T.z * R[1][2] - T.y * R[2][2]) > ra + rb) return false;

		ra = a.extents.x * AbsR[2][0] + a.extents.z * AbsR[0][0];
		rb = b.extents.y * AbsR[1][2] + b.extents.z * AbsR[1][1];
		if (std::abs(T.x * R[2][0] - T.z * R[0][0]) > ra + rb) return false;

		ra = a.extents.x * AbsR[2][1] + a.extents.z * AbsR[0][1];
		rb = b.extents.x * AbsR[1][2] + b.extents.z * AbsR[1][0];
		if (std::abs(T.x * R[2][1] - T.z * R[0][1]) > ra + rb) return false;

		ra = a.extents.x * AbsR[2][2] + a.extents.z * AbsR[0][2];
		rb = b.extents.x * AbsR[1][1] + b.extents.y * AbsR[1][0];
		if (std::abs(T.x * R[2][2] - T.z * R[0][2]) > ra + rb) return false;

		ra = a.extents.x * AbsR[1][0] + a.extents.y * AbsR[0][0];
		rb = b.extents.y * AbsR[2][2] + b.extents.z * AbsR[2][1];
		if (std::abs(T.y * R[0][0] - T.x * R[1][0]) > ra + rb) return false;

		ra = a.extents.x * AbsR[1][1] + a.extents.y * AbsR[0][1];
		rb = b.extents.x * AbsR[2][2] + b.extents.z * AbsR[2][0];
		if (std::abs(T.y * R[0][1] - T.x * R[1][1]) > ra + rb) return false;

		ra = a.extents.x * AbsR[1][2] + a.extents.y * AbsR[0][2];
		rb = b.extents.x * AbsR[2][1] + b.extents.y * AbsR[2][0];
		if (std::abs(T.y * R[0][2] - T.x * R[1][2]) > ra + rb) return false;

		return true;
	}

	// -------------------------------------------------------------
	// レイキャスト (Ray vs Shapes)
	// HitInfo を受け取るオーバーロードと bool のみの両方を用意
	// -------------------------------------------------------------

	// Ray vs BSphere
	static bool IsCollision(const Ray& ray, const BSphere& sphere, HitInfo* outHit = nullptr) {
		Vector3 m = ray.origin - sphere.center;
		float b = Vector3::dot(m, ray.direction);
		float c = Vector3::dot(m, m) - sphere.radius * sphere.radius;

		if (c > 0.0f && b > 0.0f) return false;

		float discriminant = b * b - c;
		if (discriminant < 0.0f) return false;

		if (outHit) {
			float t = -b - std::sqrt(discriminant);
			if (t < 0.0f) t = 0.0f;

			outHit->hit = true;
			outHit->distance = t;
			outHit->point = ray.origin + ray.direction * t;
			outHit->normal = (outHit->point - sphere.center).normalized();
		}
		return true;
	}

	// Ray vs AABB
	static bool IsCollision(const Ray& ray, const AABB& box, HitInfo* outHit = nullptr) {
		float tMin = 0.0f;
		float tMax = (std::numeric_limits<float>::max)();

		for (int i = 0; i < 3; ++i) {
			float origin = (i == 0) ? ray.origin.x : (i == 1) ? ray.origin.y : ray.origin.z;
			float dir = (i == 0) ? ray.direction.x : (i == 1) ? ray.direction.y : ray.direction.z;
			float bMin = (i == 0) ? box.min.x : (i == 1) ? box.min.y : box.min.z;
			float bMax = (i == 0) ? box.max.x : (i == 1) ? box.max.y : box.max.z;

			if (std::abs(dir) < 1e-6f) {
				if (origin < bMin || origin > bMax) return false;
			}
			else {
				float invD = 1.0f / dir;
				float t1 = (bMin - origin) * invD;
				float t2 = (bMax - origin) * invD;

				if (t1 > t2) std::swap(t1, t2);

				tMin = (std::max)(tMin, t1);
				tMax = (std::min)(tMax, t2);

				if (tMin > tMax) return false;
			}
		}

		if (outHit) {
			outHit->hit = true;
			outHit->distance = tMin;
			outHit->point = ray.origin + ray.direction * tMin;
		}
		return true;
	}


} // namespace Physics3D