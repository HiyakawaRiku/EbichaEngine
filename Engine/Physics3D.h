#pragma once
#include "EMath.h"

namespace Physics3D {

    // =================================================================
    // 衝突プリミティブ定義
    // =================================================================

    // 1. AABB (Axis-Aligned Bounding Box: 軸平行境界ボックス)
    struct AABB {
        Vector3 min;
        Vector3 max;

        // 点を包含するように拡張する便利関数
        void encapsulate(const Vector3& point) {
            min.x = std::min(min.x, point.x);
            min.y = std::min(min.y, point.y);
            min.z = std::min(min.z, point.z);
            max.x = std::max(max.x, point.x);
            max.y = std::max(max.y, point.y);
            max.z = std::max(max.z, point.z);
        }
    };

    // 2. BSphere (Bounding Sphere: 境界球)
    struct BSphere {
        Vector3 center;
        float radius{ 1.0f };
    };

    // 3. Ray (レイ / 光線)
    struct Ray {
        Vector3 origin;
        Vector3 direction{ 0.0f, 0.0f, 1.0f }; // 正規化推奨
    };

    // 4. Plane (平面: ax + by + cz + d = 0)
    struct Plane {
        Vector3 normal{ 0.0f, 1.0f, 0.0f }; // 法線ベクトル（正規化推奨）
        float distance{ 0.0f };             // 原点からの距離
    };

    // 5. OBB (Oriented Bounding Box: 有向境界ボックス)
    struct OBB {
        Vector3 center;
        Vector3 extents; // 各軸の半分の長さ (Half-widths)
        Matrix4x4 transform; // 回転と位置を含む変換行列（軸抽出に使用）

        Vector3 getAxis(int index) const {
            return transform.getAxis(index).normalized();
        }
    };

    // 衝突交差情報構造体（Raycastなどの結果格納用）
    struct HitInfo {
        bool hit{ false };
        float distance{ 0.0f };
        Vector3 point;
        Vector3 normal;
    };

    // =================================================================
    // 衝突判定クラス (Intersection Tests)
    // =================================================================

    class Collision {
    public:

        // -------------------------------------------------------------
        // 球 vs 球 (BSphere vs BSphere)
        // -------------------------------------------------------------
        static bool TestSphereSphere(const BSphere& a, const BSphere& b) {
            float distanceSq = (a.center - b.center).lengthSq();
            float radiusSum = a.radius + b.radius;
            return distanceSq <= (radiusSum * radiusSum);
        }

        // -------------------------------------------------------------
        // AABB vs AABB
        // -------------------------------------------------------------
        static bool TestAABBAABB(const AABB& a, const AABB& b) {
            if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
            if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
            if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
            return true;
        }

        // -------------------------------------------------------------
        // 球 vs AABB (Sphere vs AABB)
        // -------------------------------------------------------------
        static bool TestSphereAABB(const BSphere& sphere, const AABB& box) {
            // AABB上で球の中心に最も近い点を計算
            float closestX = std::clamp(sphere.center.x, box.min.x, box.max.x);
            float closestY = std::clamp(sphere.center.y, box.min.y, box.max.y);
            float closestZ = std::clamp(sphere.center.z, box.min.z, box.max.z);

            Vector3 closestPoint{ closestX, closestY, closestZ };
            float distanceSq = (sphere.center - closestPoint).lengthSq();

            return distanceSq <= (sphere.radius * sphere.radius);
        }

        // -------------------------------------------------------------
        // Ray vs 球 (Ray vs BSphere)
        // -------------------------------------------------------------
        static HitInfo IntersectRaySphere(const Ray& ray, const BSphere& sphere) {
            HitInfo hit;
            Vector3 m = ray.origin - sphere.center;
            float b = Vector3::dot(m, ray.direction);
            float c = Vector3::dot(m, m) - sphere.radius * sphere.radius;

            // レイの始点が球の外にあり、球から離れる方向を向いている場合
            if (c > 0.0f && b > 0.0f) return hit;

            float discriminant = b * b - c;
            // 判別式が負なら交差しない
            if (discriminant < 0.0f) return hit;

            float t = -b - std::sqrt(discriminant);
            if (t < 0.0f) t = 0.0f; // レイの始点が球の内部にある場合

            hit.hit = true;
            hit.distance = t;
            hit.point = ray.origin + ray.direction * t;
            hit.normal = (hit.point - sphere.center).normalized();
            return hit;
        }

        // -------------------------------------------------------------
        // Ray vs AABB (Slab Algorithm)
        // -------------------------------------------------------------
        static HitInfo IntersectRayAABB(const Ray& ray, const AABB& box) {
            HitInfo hit;
            float tMin = 0.0f;
            float tMax = std::numeric_limits<float>::max();

            // 3軸（X, Y, Z）についてスラブレースを実行
            for (int i = 0; i < 3; ++i) {
                float origin = (i == 0) ? ray.origin.x : (i == 1) ? ray.origin.y : ray.origin.z;
                float dir = (i == 0) ? ray.direction.x : (i == 1) ? ray.direction.y : ray.direction.z;
                float bMin = (i == 0) ? box.min.x : (i == 1) ? box.min.y : box.min.z;
                float bMax = (i == 0) ? box.max.x : (i == 1) ? box.max.y : box.max.z;

                if (std::abs(dir) < 1e-6f) {
                    // レイが軸に平行な場合、範囲外なら交差しない
                    if (origin < bMin || origin > bMax) return hit;
                }
                else {
                    float invD = 1.0f / dir;
                    float t1 = (bMin - origin) * invD;
                    float t2 = (bMax - origin) * invD;

                    if (t1 > t2) std::swap(t1, t2);

                    tMin = std::max(tMin, t1);
                    tMax = std::min(tMax, t2);

                    if (tMin > tMax) return hit;
                }
            }

            hit.hit = true;
            hit.distance = tMin;
            hit.point = ray.origin + ray.direction * tMin;
            return hit;
        }

        // -------------------------------------------------------------
        // OBB vs OBB (分離軸定理: SAT - Separating Axis Theorem)
        // -------------------------------------------------------------
        static bool TestOBBOBB(const OBB& a, const OBB& b) {
            float ra, rb;
            float R[3][3], AbsR[3][3];

            // 各OBBのローカル3軸を取得
            Vector3 A[3] = { a.getAxis(0), a.getAxis(1), a.getAxis(2) };
            Vector3 B[3] = { b.getAxis(0), b.getAxis(1), b.getAxis(2) };

            // 中心間ベクトルを計算し、Aのローカル空間に変換
            Vector3 v = b.center - a.center;
            Vector3 T = { Vector3::dot(v, A[0]), Vector3::dot(v, A[1]), Vector3::dot(v, A[2]) };

            // 回転行列の計算
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    R[i][j] = Vector3::dot(A[i], B[j]);
                    AbsR[i][j] = std::abs(R[i][j]) + 1e-4f; // 平行時の浮動小数点誤差対策
                }
            }

            // 15の分離軸をチェック

            // 軸: A0, A1, A2
            for (int i = 0; i < 3; i++) {
                ra = (i == 0) ? a.extents.x : (i == 1) ? a.extents.y : a.extents.z;
                rb = b.extents.x * AbsR[i][0] + b.extents.y * AbsR[i][1] + b.extents.z * AbsR[i][2];
                if (std::abs((i == 0) ? T.x : (i == 1) ? T.y : T.z) > ra + rb) return false;
            }

            // 軸: B0, B1, B2
            for (int i = 0; i < 3; i++) {
                ra = a.extents.x * AbsR[0][i] + a.extents.y * AbsR[1][i] + a.extents.z * AbsR[2][i];
                rb = (i == 0) ? b.extents.x : (i == 1) ? b.extents.y : b.extents.z;
                float t = std::abs(T.x * R[0][i] + T.y * R[1][i] + T.z * R[2][i]);
                if (t > ra + rb) return false;
            }

            // 外積による9つの複合軸 (A0xB0, A0xB1, ... A2xB2)
            // A0 x B0
            ra = a.extents.y * AbsR[2][0] + a.extents.z * AbsR[1][0];
            rb = b.extents.y * AbsR[0][2] + b.extents.z * AbsR[0][1];
            if (std::abs(T.z * R[1][0] - T.y * R[2][0]) > ra + rb) return false;

            // A0 x B1
            ra = a.extents.y * AbsR[2][1] + a.extents.z * AbsR[1][1];
            rb = b.extents.x * AbsR[0][2] + b.extents.z * AbsR[0][0];
            if (std::abs(T.z * R[1][1] - T.y * R[2][1]) > ra + rb) return false;

            // A0 x B2
            ra = a.extents.y * AbsR[2][2] + a.extents.z * AbsR[1][2];
            rb = b.extents.x * AbsR[0][1] + b.extents.y * AbsR[0][0];
            if (std::abs(T.z * R[1][2] - T.y * R[2][2]) > ra + rb) return false;

            // A1 x B0
            ra = a.extents.x * AbsR[2][0] + a.extents.z * AbsR[0][0];
            rb = b.extents.y * AbsR[1][2] + b.extents.z * AbsR[1][1];
            if (std::abs(T.x * R[2][0] - T.z * R[0][0]) > ra + rb) return false;

            // A1 x B1
            ra = a.extents.x * AbsR[2][1] + a.extents.z * AbsR[0][1];
            rb = b.extents.x * AbsR[1][2] + b.extents.z * AbsR[1][0];
            if (std::abs(T.x * R[2][1] - T.z * R[0][1]) > ra + rb) return false;

            // A1 x B2
            ra = a.extents.x * AbsR[2][2] + a.extents.z * AbsR[0][2];
            rb = b.extents.x * AbsR[1][1] + b.extents.y * AbsR[1][0];
            if (std::abs(T.x * R[2][2] - T.z * R[0][2]) > ra + rb) return false;

            // A2 x B0
            ra = a.extents.x * AbsR[1][0] + a.extents.y * AbsR[0][0];
            rb = b.extents.y * AbsR[2][2] + b.extents.z * AbsR[2][1];
            if (std::abs(T.y * R[0][0] - T.x * R[1][0]) > ra + rb) return false;

            // A2 x B1
            ra = a.extents.x * AbsR[1][1] + a.extents.y * AbsR[0][1];
            rb = b.extents.x * AbsR[2][2] + b.extents.z * AbsR[2][0];
            if (std::abs(T.y * R[0][1] - T.x * R[1][1]) > ra + rb) return false;

            // A2 x B2
            ra = a.extents.x * AbsR[1][2] + a.extents.y * AbsR[0][2];
            rb = b.extents.x * AbsR[2][1] + b.extents.y * AbsR[2][0];
            if (std::abs(T.y * R[0][2] - T.x * R[1][2]) > ra + rb) return false;

            // 分離軸が見つからなければ交差している
            return true;
        }
    };

} // namespace Physics3D