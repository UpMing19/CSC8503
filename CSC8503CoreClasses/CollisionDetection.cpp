#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Window.h"
#include "Maths.h"
#include "Debug.h"
#include <algorithm>
#include "Vector3.h"

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray &r, const Plane &p, RayCollision &collisions) {
    float ln = Vector3::Dot(p.GetNormal(), r.GetDirection());

    if (ln == 0.0f) {
        return false; //direction vectors are perpendicular!
    }

    Vector3 planePoint = p.GetPointOnPlane();

    Vector3 pointDir = planePoint - r.GetPosition();

    float d = Vector3::Dot(pointDir, p.GetNormal()) / ln;

    collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);

    return true;
}

bool CollisionDetection::RayIntersection(const Ray &r, GameObject &object, RayCollision &collision) {
    bool hasCollided = false;

    const Transform &worldTransform = object.GetTransform();
    const CollisionVolume *volume = object.GetBoundingVolume();

    if (!volume) {
        return false;
    }

    switch (volume->type) {
        case VolumeType::AABB:
            hasCollided = RayAABBIntersection(r, worldTransform, (const AABBVolume &) *volume, collision);
            break;
        case VolumeType::OBB:
            hasCollided = RayOBBIntersection(r, worldTransform, (const OBBVolume &) *volume, collision);
            break;
        case VolumeType::Sphere:
            hasCollided = RaySphereIntersection(r, worldTransform, (const SphereVolume &) *volume, collision);
            break;

        case VolumeType::Capsule:
            hasCollided = RayCapsuleIntersection(r, worldTransform, (const CapsuleVolume &) *volume, collision);
            break;
    }

    return hasCollided;
}

bool CollisionDetection::RayBoxIntersection(const Ray &r, const Vector3 &boxPos, const Vector3 &boxSize,
                                            RayCollision &collision) {
    Vector3 boxMin = boxPos - boxSize;
    Vector3 boxMax = boxPos + boxSize;

    Vector3 rayPos = r.GetPosition();
    Vector3 rayDir = r.GetDirection();

    Vector3 tVals(-1, -1, -1);

    for (int i = 0; i < 3; ++i) { // get best 3 intersections
        if (rayDir[i] > 0) {
            tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i];
        } else if (rayDir[i] < 0) {
            tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
        }
    }
    float bestT = tVals.GetMaxElement();
    if (bestT < 0.0f) {
        return false; // no backwards ray!
    }
    Vector3 intersection = rayPos + (rayDir * bestT);
    const float epsilon = 0.0001f; // an amount of leeway in our calcs
    for (int i = 0; i < 3; ++i) {
        if (intersection[i] + epsilon < boxMin[i] || intersection[i] - epsilon > boxMax[i]) {
            return false; // best intersection doesn’t touch the box !
        }
    }
    collision.collidedAt = intersection;
    collision.rayDistance = bestT;
    return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray &r, const Transform &worldTransform, const AABBVolume &volume,
                                             RayCollision &collision) {

    Vector3 boxPos = worldTransform.GetPosition();
    Vector3 boxSize = volume.GetHalfDimensions();

    return RayBoxIntersection(r, boxPos, boxSize, collision);

}

bool CollisionDetection::RayOBBIntersection(const Ray &r, const Transform &worldTransform, const OBBVolume &volume,
                                            RayCollision &collision) {
    Quaternion orientation = worldTransform.GetOrientation();
    Vector3 position = worldTransform.GetPosition();

    Matrix3 transform = Matrix3(orientation);
    Matrix3 invTransform = Matrix3(orientation.Conjugate());

    Vector3 localRayPos = r.GetPosition() - position;

    Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());

    bool collided = RayBoxIntersection(tempRay, Vector3(), volume.GetHalfDimensions(), collision);

    if (collided) {
        collision.collidedAt = transform * collision.collidedAt + position;
        collision.collidedNormal = transform * collision.collidedNormal;
    }
    return collided;
}

bool
CollisionDetection::RaySphereIntersection(const Ray &r, const Transform &worldTransform, const SphereVolume &volume,
                                          RayCollision &collision) {

    Vector3 spherePos = worldTransform.GetPosition();
    float sphereRadius = volume.GetRadius();

    Vector3 dir = (spherePos - r.GetPosition());

    float sphereProj = Vector3::Dot(dir, r.GetDirection());

    if (sphereProj < 0.0f) return false;

    Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);

    float sphereDist = (point - spherePos).Length();

    if (sphereDist > sphereRadius) return false;

    float offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));

    collision.rayDistance = sphereProj - (offset);
    collision.collidedAt = r.GetPosition() + (r.GetDirection() * collision.rayDistance);
    return true;

}

// Using Ray-Plane to get some coordinates for where the ray has hit the capsule - then place a sphere and do Ray-Sphere on it
bool CollisionDetection::RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision) {
    Vector3 firstPoint = worldTransform.GetPosition() + (worldTransform.GetOrientation() * (Vector3(0, 1, 0) * (volume.GetHalfHeight() - volume.GetRadius())));
    Vector3 secondPoint = worldTransform.GetPosition() - (worldTransform.GetOrientation() * (Vector3(0, 1, 0) * (volume.GetHalfHeight() - volume.GetRadius())));

    // Cross product between the capsules center and the normal
    Vector3 c = Vector3::Cross(worldTransform.GetPosition(), r.GetPosition() - worldTransform.GetPosition());
    Vector3 thirdPoint = worldTransform.GetPosition() + c.Normalised();

    Plane p = Plane::PlaneFromTri(firstPoint, secondPoint, thirdPoint);
    if (!RayPlaneIntersection(r, p, collision)) return false;

    Vector3 capsuleDirection = firstPoint - secondPoint;
    float capsuleLength = capsuleDirection.Length();
    capsuleDirection.Normalise();

    Vector3 rayCapDirection = collision.collidedAt - secondPoint;
    float dotProd = Maths::Clamp(Vector3::Dot(rayCapDirection, capsuleDirection), 0.0f, capsuleLength);

    // Create sphere for Ray-Sphere check
    SphereVolume sphere(volume.GetRadius());
    Transform sphereTransform;
    sphereTransform.SetPosition(secondPoint + (capsuleDirection * dotProd));
    sphereTransform.SetScale(Vector3(volume.GetRadius(), volume.GetRadius(), volume.GetRadius()));
    return RaySphereIntersection(r, sphereTransform, sphere, collision);
}
bool CollisionDetection::ObjectIntersection(GameObject *a, GameObject *b, CollisionInfo &collisionInfo) {
    const CollisionVolume *volA = a->GetBoundingVolume();
    const CollisionVolume *volB = b->GetBoundingVolume();

    if (!volA || !volB) {
        return false;
    }

    collisionInfo.a = a;
    collisionInfo.b = b;

    Transform &transformA = a->GetTransform();
    Transform &transformB = b->GetTransform();

    VolumeType pairType = (VolumeType) ((int) volA->type | (int) volB->type);

    //Two AABBs
    if (pairType == VolumeType::AABB) {
        return AABBIntersection((AABBVolume &) *volA, transformA, (AABBVolume &) *volB, transformB, collisionInfo);
    }
    //Two Spheres
    if (pairType == VolumeType::Sphere) {
        return SphereIntersection((SphereVolume &) *volA, transformA, (SphereVolume &) *volB, transformB,
                                  collisionInfo);
    }
    //Two OBBs
    if (pairType == VolumeType::OBB) {
        return OBBIntersection((OBBVolume &) *volA, transformA, (OBBVolume &) *volB, transformB, collisionInfo);
    }
    //Two Capsules
    if (pairType == VolumeType::Capsule) {
        return CapsuleIntersection((CapsuleVolume&)*volA, transformA, (CapsuleVolume&)*volB, transformB, collisionInfo);
    }

    //AABB vs Sphere pairs
    if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
        return AABBSphereIntersection((AABBVolume &) *volA, transformA, (SphereVolume &) *volB, transformB,
                                      collisionInfo);
    }
    if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
        collisionInfo.a = b;
        collisionInfo.b = a;
        return AABBSphereIntersection((AABBVolume &) *volB, transformB, (SphereVolume &) *volA, transformA,
                                      collisionInfo);
    }

    //OBB vs sphere pairs
    if (volA->type == VolumeType::OBB && volB->type == VolumeType::Sphere) {
        return OBBSphereIntersection((OBBVolume &) *volA, transformA, (SphereVolume &) *volB, transformB,
                                     collisionInfo);
    }
    if (volA->type == VolumeType::Sphere && volB->type == VolumeType::OBB) {
        collisionInfo.a = b;
        collisionInfo.b = a;
        return OBBSphereIntersection((OBBVolume &) *volB, transformB, (SphereVolume &) *volA, transformA,
                                     collisionInfo);
    }

    //Capsule vs other interactions
    if (volA->type == VolumeType::Capsule && volB->type == VolumeType::Sphere) {
        return SphereCapsuleIntersection((CapsuleVolume &) *volA, transformA, (SphereVolume &) *volB, transformB,
                                         collisionInfo);
    }
    if (volA->type == VolumeType::Sphere && volB->type == VolumeType::Capsule) {
        collisionInfo.a = b;
        collisionInfo.b = a;
        return SphereCapsuleIntersection((CapsuleVolume &) *volB, transformB, (SphereVolume &) *volA, transformA,
                                         collisionInfo);
    }

    if (volA->type == VolumeType::Capsule && volB->type == VolumeType::AABB) {
        return AABBCapsuleIntersection((AABBVolume&)*volB, transformB, (CapsuleVolume&)*volA, transformA, collisionInfo);
    }
    if (volA->type == VolumeType::AABB && volB->type == VolumeType::Capsule) {
        collisionInfo.a = b;
        collisionInfo.b = a;
        return AABBCapsuleIntersection((AABBVolume&)*volA, transformA, (CapsuleVolume&)*volB, transformB, collisionInfo);
    }

    return false;
}

bool CollisionDetection::AABBTest(const Vector3 &posA, const Vector3 &posB, const Vector3 &halfSizeA,
                                  const Vector3 &halfSizeB) {
    Vector3 delta = posB - posA;
    Vector3 totalSize = halfSizeA + halfSizeB;

    if (abs(delta.x) < totalSize.x &&
        abs(delta.y) < totalSize.y &&
        abs(delta.z) < totalSize.z) {
        return true;
    }
    return false;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume &volumeA, const Transform &worldTransformA,
                                          const AABBVolume &volumeB, const Transform &worldTransformB,
                                          CollisionInfo &collisionInfo) {

    Vector3 boxAPos = worldTransformA.GetPosition();
    Vector3 boxBPos = worldTransformB.GetPosition();

    Vector3 boxASize = volumeA.GetHalfDimensions();
    Vector3 boxBSize = volumeB.GetHalfDimensions();

    bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);

    if (overlap) {
        static const Vector3 faces[6] =
                {
                        Vector3(-1, 0, 0), Vector3(1, 0, 0),
                        Vector3(0, -1, 0), Vector3(0, 1, 0),
                        Vector3(0, 0, -1), Vector3(0, 0, 1),
                };

        Vector3 maxA = boxAPos + boxASize;
        Vector3 minA = boxAPos - boxASize;

        Vector3 maxB = boxBPos + boxBSize;
        Vector3 minB = boxBPos - boxBSize;

        float distances[6] =
                {
                        (maxB.x - minA.x),// distance of box ’b’ to ’left ’ of ’a ’.
                        (maxA.x - minB.x),// distance of box ’b’ to ’right ’ of ’a ’.
                        (maxB.y - minA.y),// distance of box ’b’ to ’bottom ’ of ’a ’.
                        (maxA.y - minB.y),// distance of box ’b’ to ’top ’ of ’a ’.
                        (maxB.z - minA.z),// distance of box ’b’ to ’far ’ of ’a ’.
                        (maxA.z - minB.z) // distance of box ’b’ to ’near ’ of ’a ’.
                };
        float penetration = FLT_MAX;
        Vector3 bestAxis;

        for (int i = 0; i < 6; i++) {
            if (distances[i] < penetration) {
                penetration = distances[i];
                bestAxis = faces[i];
            }
        }
        collisionInfo.AddContactPoint(Vector3(), Vector3(),
                                      bestAxis, penetration);
        return true;
    }

    return false;
}

//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume &volumeA, const Transform &worldTransformA,
                                            const SphereVolume &volumeB, const Transform &worldTransformB,
                                            CollisionInfo &collisionInfo) {


    float radii = volumeA.GetRadius() + volumeB.GetRadius();
    Vector3 delta = worldTransformB.GetPosition() -
                    worldTransformA.GetPosition();

    float deltaLength = delta.Length();

    if (deltaLength < radii) {
        float penetration = (radii - deltaLength);
        Vector3 normal = delta.Normalised();
        Vector3 localA = normal * volumeA.GetRadius();
        Vector3 localB = -normal * volumeB.GetRadius();

        collisionInfo.AddContactPoint(localA, localB, normal, penetration);
        return true;// we ’re colliding !
    }
    return false;

}

//AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(const AABBVolume &volumeA, const Transform &worldTransformA,
                                                const SphereVolume &volumeB, const Transform &worldTransformB,
                                                CollisionInfo &collisionInfo) {


    Vector3 boxSize = volumeA.GetHalfDimensions();
    Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();
    // Vector3 closestPointOnBox = NCL::Maths::Vector3::Clamp(delta, -boxSize, boxSize);


/*    Vector3 closestPointOnBox(
            std::clamp(delta.x, -boxSize.x, boxSize.x),
            std::clamp(delta.y, -boxSize.y, boxSize.y),
            std::clamp(delta.z, -boxSize.z, boxSize.z)
    );*/

    Vector3 closestPointOnBox = [&]() -> Vector3 {
        return Vector3(
                std::clamp(delta.x, -boxSize.x, boxSize.x),
                std::clamp(delta.y, -boxSize.y, boxSize.y),
                std::clamp(delta.z, -boxSize.z, boxSize.z)
        );
    }();


    Vector3 localPoint = delta - closestPointOnBox;
    float distance = localPoint.Length();

    if (distance < volumeB.GetRadius()) {// yes , we ’re colliding !
        Vector3 collisionNormal = localPoint.Normalised();
        float penetration = (volumeB.GetRadius() - distance);

        Vector3 localA = Vector3();
        Vector3 localB = -collisionNormal * volumeB.GetRadius();

        collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
        return true;
    }
    return false;
}

bool CollisionDetection::CapsuleIntersection(	const CapsuleVolume& volumeA, const Transform& worldTransformA,
                                                 const CapsuleVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
    // Capsule A
    Vector3 capsuleTopA = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * (Vector3(0, 1, 0) * (volumeA.GetHalfHeight() - volumeA.GetRadius())));
    Vector3 capsuleBottomA = worldTransformA.GetPosition() - (worldTransformA.GetOrientation() * (Vector3(0, 1, 0) * (volumeA.GetHalfHeight() - volumeA.GetRadius())));
    Vector3 capsuleDirectionA = capsuleTopA - capsuleBottomA;
    float capsuleLengthA = capsuleDirectionA.Length();
    capsuleDirectionA.Normalise();

    // Capsule B
    Vector3 capsuleTopB = worldTransformB.GetPosition() + (worldTransformB.GetOrientation() * (Vector3(0, 1, 0) * (volumeB.GetHalfHeight() - volumeB.GetRadius())));
    Vector3 capsuleBottomB = worldTransformB.GetPosition() - (worldTransformB.GetOrientation() * (Vector3(0, 1, 0) * (volumeB.GetHalfHeight() - volumeB.GetRadius())));
    Vector3 capsuleDirectionB = capsuleTopB - capsuleBottomB;
    float capsuleLengthB = capsuleDirectionB.Length();
    capsuleDirectionB.Normalise();

    Vector3 v0 = capsuleTopB - capsuleTopA;
    Vector3 v1 = capsuleBottomB - capsuleTopA;
    Vector3 v2 = capsuleTopB - capsuleBottomA;
    Vector3 v3 = capsuleBottomB - capsuleBottomA;

    float d0 = Vector3::Dot(v0, v0);
    float d1 = Vector3::Dot(v1, v1);
    float d2 = Vector3::Dot(v2, v2);
    float d3 = Vector3::Dot(v3, v3);

    Vector3 bestA;
    if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1) {
        bestA = capsuleBottomA;
    }
    else {
        bestA = capsuleTopA;
    }

    return false;
}
bool CollisionDetection::OBBSphereIntersection(const OBBVolume &volumeA, const Transform &worldTransformA,
                                               const SphereVolume &volumeB, const Transform &worldTransformB,
                                               CollisionInfo &collisionInfo) {

    Matrix4 boxMat = worldTransformA.GetMatrix();
    //Debug::DrawAxisLines(boxMat, 3.0f, 1000.0);
    boxMat.SetPositionVector(Vector3(0, 0, 0));

    Vector3 boxRight = boxMat * Vector3(1, 0, 0);
    Vector3 boxUp = boxMat * Vector3(0, 1, 0);
    Vector3 boxForward = boxMat * Vector3(0, 0, -1);

    Vector3 u[3] = { boxRight.Normalised(), boxUp.Normalised(), boxForward.Normalised() };

    Vector3 boxCenter = worldTransformA.GetPosition();
    Vector3 boxSize = volumeA.GetHalfDimensions();

    Vector3 sphereCenter = worldTransformB.GetPosition();

    //Closest Point
    Vector3 d = sphereCenter - boxCenter;
    Vector3 closestPoint = boxCenter;

    for (int i = 0; i < 3; i++)
    {
        float dist = Vector3::Dot(d, u[i]);
        if (dist > boxSize[i]) dist = boxSize[i];
        if (dist < -boxSize[i]) dist = -boxSize[i];

        closestPoint = closestPoint + (u[i] * dist);
    }

    Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();
    Vector3 localPoint = delta - closestPoint;

    //Debug::DrawLine(closestPoint, closestPoint + Vector3(0, 1, 0), Debug::MAGENTA, 1000.0f);

    Vector3 v = closestPoint - sphereCenter;
    float radiusSq = volumeB.GetRadius() * volumeB.GetRadius();
    if (v.LengthSquared() <= radiusSq)
    {
        Vector3 collisionNormal = (sphereCenter - closestPoint).Normalised();
        float penetration = (volumeB.GetRadius() - v.Length());

        Vector3 localA = Vector3();
        Vector3 localB = -collisionNormal * volumeB.GetRadius();

        collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);

        return true;
    }

    return false;
}

bool CollisionDetection::AABBCapsuleIntersection(const AABBVolume& volumeB, const Transform& worldTransformB,
                                                 const CapsuleVolume& volumeA, const Transform& worldTransformA, CollisionInfo& collisionInfo) {


        Vector3 capsulePosition = worldTransformA.GetPosition();
        Quaternion orientation = worldTransformA.GetOrientation();

        Matrix3 transform = Matrix3(orientation);
        Vector3 upVector = transform * Vector3(0, 1, 0);

        Vector3 topPos = capsulePosition + (upVector * (volumeA.GetHalfHeight() - volumeA.GetRadius()));

        Vector3 localPos = worldTransformB.GetPosition() - capsulePosition;
        Vector3 originTop = topPos - capsulePosition;

        Vector3 thirdPoint = Vector3::Cross(localPos.Normalised(), originTop.Normalised());

        thirdPoint += capsulePosition;

        Vector3 vA = topPos - worldTransformB.GetPosition();
        Vector3 vB = topPos - capsulePosition;

        float dotDist = Vector3::Dot(vA, vB);

        if (dotDist < 0) {
            Vector3 penetrationDistance = worldTransformB.GetPosition() - topPos;

            float penetrationFloat = penetrationDistance.Length();

            if (penetrationFloat <= volumeA.GetRadius() + volumeB.GetHalfDimensions().Length()) {
                SphereVolume sphere = SphereVolume(volumeA.GetRadius());
                Transform transform = Transform();
                transform.SetPosition(topPos);
                transform.SetOrientation(orientation);
                transform.SetScale(worldTransformA.GetScale());
                GameObject* a = collisionInfo.a;
                GameObject* b = collisionInfo.b;

                collisionInfo.a = b;
                collisionInfo.b = a;

                return AABBSphereIntersection(volumeB, worldTransformB, sphere, transform, collisionInfo);
            }

            //return false;
        }

        Vector3 bottomPos = capsulePosition - (upVector * (volumeA.GetHalfHeight() - volumeA.GetRadius()));
        vA = bottomPos - worldTransformB.GetPosition();
        //vA.Normalise();
        vB = bottomPos - capsulePosition;
        //vB.Normalise();
        dotDist = Vector3::Dot(vA, vB);

        if (dotDist < 0) {
            Vector3 penetrationDistance = worldTransformB.GetPosition() - bottomPos;

            float penetrationFloat = penetrationDistance.Length();

            if (penetrationFloat <= volumeA.GetRadius() + volumeB.GetHalfDimensions().Length()) {
                SphereVolume sphere = SphereVolume(volumeA.GetRadius());
                Transform transform = Transform();
                transform.SetPosition(bottomPos);
                transform.SetOrientation(orientation);
                transform.SetScale(worldTransformA.GetScale());
                GameObject* a = collisionInfo.a;
                GameObject* b = collisionInfo.b;

                collisionInfo.a = b;
                collisionInfo.b = a;
                return AABBSphereIntersection(volumeB, worldTransformB, sphere, transform, collisionInfo);
            }

            //return false;
        }

        Vector3 cD = worldTransformB.GetPosition() - capsulePosition;
        //Vector3 extents = Vector3(volume.GetRadius(), volume.GetHalfHeight(), volume.GetRadius());
        float clampedDot = Maths::Clamp(Vector3::Dot(cD, upVector), -volumeA.GetHalfHeight(), volumeA.GetHalfHeight());

        Vector3 d = capsulePosition + (upVector * (clampedDot));

        float dist = (d - worldTransformB.GetPosition()).Length();

        if (dist > volumeA.GetRadius() + volumeB.GetHalfDimensions().Length()) {
            return false;
        }

            //if (dist > volumeA.GetRadius() + volumeB.GetHalfDimensions().Length()) {
            //	return false;
            //}

        else {
            SphereVolume sphere = SphereVolume(volumeA.GetRadius());
            Transform transform = Transform();
            transform.SetPosition(d);
            transform.SetOrientation(orientation);
            transform.SetScale(worldTransformA.GetScale());
            GameObject* a = collisionInfo.a;
            GameObject* b = collisionInfo.b;

            collisionInfo.a = b;
            collisionInfo.b = a;
            return AABBSphereIntersection(volumeB, worldTransformB, sphere, transform, collisionInfo);
            //return true;
        }

        return false;
}

// Sphere-Capsule by Sphere-Sphere
bool CollisionDetection::SphereCapsuleIntersection(
        const CapsuleVolume& volumeA, const Transform& worldTransformA,
        const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

    Vector3 capsuleTop = worldTransformA.GetPosition() + (worldTransformA.GetOrientation() * (Vector3(0, 1, 0) * (volumeA.GetHalfHeight() - volumeA.GetRadius())));
    Vector3 capsuleBottom = worldTransformA.GetPosition() - (worldTransformA.GetOrientation() * (Vector3(0, 1, 0) * (volumeA.GetHalfHeight() - volumeA.GetRadius())));

    Vector3 capsuleDirection = capsuleTop - capsuleBottom;
    float capsuleLength = capsuleDirection.Length();
    capsuleDirection.Normalise();

    Vector3 rayCapDirection = worldTransformB.GetPosition() - capsuleBottom;
    float dotProd = Maths::Clamp(Vector3::Dot(rayCapDirection, capsuleDirection), 0.0f, capsuleLength);

    // Create sphere for Ray-Sphere check
    SphereVolume sphereFromCapsule(volumeA.GetRadius());
    Transform sphereFromCapsuleTransform;
    sphereFromCapsuleTransform.SetPosition(capsuleBottom + (capsuleDirection * dotProd));
    sphereFromCapsuleTransform.SetScale(Vector3(volumeA.GetRadius(), volumeA.GetRadius(), volumeA.GetRadius()));
    return SphereIntersection(sphereFromCapsule, sphereFromCapsuleTransform, volumeB, worldTransformB, collisionInfo);
}

//https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
bool CollisionDetection::OBBIntersection(const OBBVolume &volumeA, const Transform &worldTransformA,
                                         const OBBVolume &volumeB, const Transform &worldTransformB,
                                         CollisionInfo &collisionInfo) {
    float ra, rb;
    Matrix3 R, AbsR;

    //------------------------------------------------------------------------------
    //Get Local Axis of OBB A
    Matrix4 boxAMat = worldTransformA.GetMatrix();
    boxAMat.SetPositionVector(Vector3(0, 0, 0));

    Vector3 boxARight = boxAMat * Vector3(1, 0, 0);
    Vector3 boxAUp = boxAMat * Vector3(0, 1, 0);
    Vector3 boxAForward = boxAMat * Vector3(0, 0, -1);

    Vector3 uA[3] = { boxARight.Normalised(), boxAUp.Normalised(), boxAForward.Normalised() };

    Vector3 boxAExtents = volumeA.GetHalfDimensions();
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    //Get Local Axis of OBB B
    Matrix4 boxBMat = worldTransformB.GetMatrix();
    boxBMat.SetPositionVector(Vector3(0, 0, 0));

    Vector3 boxBRight = boxBMat * Vector3(1, 0, 0);
    Vector3 boxBUp = boxBMat * Vector3(0, 1, 0);
    Vector3 boxBForward = boxBMat * Vector3(0, 0, -1);

    Vector3 uB[3] = { boxBRight.Normalised(), boxBUp.Normalised(), boxBForward.Normalised() };

    Vector3 boxBExtents = volumeB.GetHalfDimensions();
    //------------------------------------------------------------------------------

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            R.array[i][j] = Vector3::Dot(uA[i], uB[j]);

    Vector3 t = worldTransformB.GetPosition() - worldTransformA.GetPosition();
    t = Vector3(Vector3::Dot(t, uA[0]), Vector3::Dot(t, uA[1]), Vector3::Dot(t, uA[2]));

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            AbsR.array[i][j] = abs(R.array[i][j]) + 0.01f;

    //Test Axes L = A0, L = A1, L = A2
    for (int i = 0; i < 3; i++)
    {
        ra = boxAExtents[i];
        rb = boxBExtents[0] * AbsR.array[i][0] +
             boxBExtents[1] * AbsR.array[i][1] +
             boxBExtents[2] * AbsR.array[i][2];

        if (abs(t[i]) > ra + rb)
            return false;
    }

    //Test Axes L = B0, L = B1, L = B2
    for (int i = 0; i < 3; i++)
    {
        ra = boxAExtents[0] * AbsR.array[0][i] +
             boxAExtents[1] * AbsR.array[1][i] +
             boxAExtents[2] * AbsR.array[2][i];
        rb = boxBExtents[i];

        if (abs(t[0] * R.array[0][i] + t[1] * R.array[1][i] + t[2] * R.array[2][i]) > ra + rb)
            return false;
    }



    //Test Axis L = A0 x B0
    ra = boxAExtents[1] * AbsR.array[2][0] + boxAExtents[2] * AbsR.array[1][0];
    rb = boxBExtents[1] * AbsR.array[0][2] + boxBExtents[2] * AbsR.array[0][1];
    if (abs(t[2] * R.array[1][0] - t[1] * R.array[2][0]) > ra + rb)
        return false;

    //Test Axis L = A0 x B1
    ra = boxAExtents[1] * AbsR.array[2][1] + boxAExtents[2] * AbsR.array[1][1];
    rb = boxBExtents[0] * AbsR.array[0][2] + boxBExtents[2] * AbsR.array[0][0];
    if (abs(t[2] * R.array[1][1] - t[1] * R.array[2][1]) > ra + rb)
        return false;

    //Test Axis L = A0 x B2
    ra = boxAExtents[1] * AbsR.array[2][2] + boxAExtents[2] * AbsR.array[1][2];
    rb = boxBExtents[0] * AbsR.array[0][1] + boxBExtents[1] * AbsR.array[0][0];
    if (abs(t[2] * R.array[1][2] - t[1] * R.array[2][2]) > ra + rb)
        return false;



    //Test Axis L = A1 x B0
    ra = boxAExtents[0] * AbsR.array[2][0] + boxAExtents[2] * AbsR.array[0][0];
    rb = boxBExtents[1] * AbsR.array[1][2] + boxBExtents[2] * AbsR.array[1][1];
    if (abs(t[0] * R.array[2][0] - t[2] * R.array[0][0]) > ra + rb)
        return false;

    //Test Axis L = A1 x B1
    ra = boxAExtents[0] * AbsR.array[2][1] + boxAExtents[2] * AbsR.array[0][1];
    rb = boxBExtents[0] * AbsR.array[1][2] + boxBExtents[2] * AbsR.array[1][0];
    if (abs(t[0] * R.array[2][1] - t[2] * R.array[0][1]) > ra + rb)
        return false;

    //Test Axis L = A1 x B2
    ra = boxAExtents[0] * AbsR.array[2][2] + boxAExtents[2] * AbsR.array[0][2];
    rb = boxBExtents[0] * AbsR.array[1][1] + boxBExtents[1] * AbsR.array[1][0];
    if (abs(t[0] * R.array[2][2] - t[2] * R.array[0][2]) > ra + rb)
        return false;




    //Test Axis L = A2 x B0
    ra = boxAExtents[0] * AbsR.array[1][0] + boxAExtents[1] * AbsR.array[0][0];
    rb = boxBExtents[1] * AbsR.array[2][2] + boxBExtents[2] * AbsR.array[2][1];
    if (abs(t[1] * R.array[0][0] - t[0] * R.array[1][0]) > ra + rb)
        return false;

    //Test Axis L = A2 x B1
    ra = boxAExtents[0] * AbsR.array[1][1] + boxAExtents[1] * AbsR.array[0][1];
    rb = boxBExtents[0] * AbsR.array[2][2] + boxBExtents[2] * AbsR.array[2][0];
    if (abs(t[1] * R.array[0][1] - t[0] * R.array[1][1]) > ra + rb)
        return false;

    //Test Axis L = A2 x B2
    ra = boxAExtents[0] * AbsR.array[1][2] + boxAExtents[1] * AbsR.array[0][2];
    rb = boxBExtents[0] * AbsR.array[2][1] + boxBExtents[1] * AbsR.array[2][0];
    if (abs(t[1] * R.array[0][2] - t[0] * R.array[1][2]) > ra + rb)
        return false;

    //Debug::DrawLine(t, t + Vector3(0, 1, 0), Debug::MAGENTA, 1000.0f);
    RayCollision rayCollision;
    Ray ray = Ray(worldTransformA.GetPosition(), worldTransformB.GetPosition() - worldTransformA.GetPosition());
    if (RayOBBIntersection(ray, worldTransformB, volumeB, rayCollision))
    {
        //Debug::DrawLine(rayCollision.collidedAt, rayCollision.collidedAt + rayCollision.collidedNormal, Debug::MAGENTA, 1000.0f);
        collisionInfo.AddContactPoint(Vector3(), rayCollision.collidedAt, -rayCollision.collidedNormal, rayCollision.rayDistance);
    }

    //std::cout << "There is an OBB-OBB collision\n";
    return true;

}

Matrix4 GenerateInverseView(const Camera &c) {
    float pitch = c.GetPitch();
    float yaw = c.GetYaw();
    Vector3 position = c.GetPosition();

    Matrix4 iview =
            Matrix4::Translation(position) *
            Matrix4::Rotation(-yaw, Vector3(0, -1, 0)) *
            Matrix4::Rotation(-pitch, Vector3(-1, 0, 0));

    return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float nearPlane, float farPlane, float fov) {
    float negDepth = nearPlane - farPlane;

    float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

    Matrix4 m;

    float h = 1.0f / tan(fov * PI_OVER_360);

    m.array[0][0] = aspect / h;
    m.array[1][1] = tan(fov * PI_OVER_360);
    m.array[2][2] = 0.0f;

    m.array[2][3] = invNegDepth;//// +PI_OVER_360;
    m.array[3][2] = -1.0f;
    m.array[3][3] = (0.5f / nearPlane) + (0.5f / farPlane);

    return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const Camera& cam) {
    Vector2i screenSize = Window::GetWindow()->GetScreenSize();

    float aspect	= screenSize.x / screenSize.y;
    float fov		= cam.GetFieldOfVision();
    float nearPlane = cam.GetNearPlane();
    float farPlane  = cam.GetFarPlane();

    //Create our inverted matrix! Note how that to get a correct inverse matrix,
    //the order of matrices used to form it are inverted, too.
    Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

    Matrix4 proj  = cam.BuildProjectionMatrix(aspect);

    //Our mouse position x and y values are in 0 to screen dimensions range,
    //so we need to turn them into the -1 to 1 axis range of clip space.
    //We can do that by dividing the mouse values by the width and height of the
    //screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
    //and then subtracting 1 (-1.0 to 1.0).
    Vector4 clipSpace = Vector4(
            (screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
            (screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
            (screenPos.z),
            1.0f
    );

    //Then, we multiply our clipspace coordinate by our inverted matrix
    Vector4 transformed = invVP * clipSpace;

    //our transformed w coordinate is now the 'inverse' perspective divide, so
    //we can reconstruct the final world space by dividing x,y,and z by w.
    return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const Camera& cam) {
    Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
    Vector2i screenSize	= Window::GetWindow()->GetScreenSize();

    //We remove the y axis mouse position from height as OpenGL is 'upside down',
    //and thinks the bottom left is the origin, instead of the top left!
    Vector3 nearPos = Vector3(screenMouse.x,
                              screenSize.y - screenMouse.y,
                              -0.99999f
    );

    //We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
    //causes the unproject function to go a bit weird.
    Vector3 farPos = Vector3(screenMouse.x,
                             screenSize.y - screenMouse.y,
                             0.99999f
    );

    Vector3 a = Unproject(nearPos, cam);
    Vector3 b = Unproject(farPos, cam);
    Vector3 c = b - a;

    c.Normalise();

    return Ray(cam.GetPosition(), c);
}

Ray CollisionDetection::BuildRayFromMouse(const Camera& cam, const Vector3& offset)
{
    Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
    Vector2i screenSize = Window::GetWindow()->GetScreenSize();

    //We remove the y axis mouse position from height as OpenGL is 'upside down',
    //and thinks the bottom left is the origin, instead of the top left!
    Vector3 nearPos = Vector3(screenMouse.x,
                              screenSize.y - screenMouse.y,
                              -0.99999f
    );

    //We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
    //causes the unproject function to go a bit weird.
    Vector3 farPos = Vector3(screenMouse.x,
                             screenSize.y - screenMouse.y,
                             0.99999f
    );

    Vector3 a = Unproject(nearPos, cam);
    Vector3 b = Unproject(farPos, cam);
    Vector3 c = b - a;

    c.Normalise();

    return Ray(cam.GetPosition() + offset, c);
}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
    Matrix4 m;

    float t = tan(fov * PI_OVER_360);

    float neg_depth = nearPlane - farPlane;

    const float h = 1.0f / t;

    float c = (farPlane + nearPlane) / neg_depth;
    float e = -1.0f;
    float d = 2.0f * (nearPlane * farPlane) / neg_depth;

    m.array[0][0] = aspect / h;
    m.array[1][1] = tan(fov * PI_OVER_360);
    m.array[2][2] = 0.0f;

    m.array[2][3] = 1.0f / d;

    m.array[3][2] = 1.0f / e;
    m.array[3][3] = -c / (d * e);

    return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
    float pitch = c.GetPitch();
    float yaw = c.GetYaw();
    Vector3 position = c.GetPosition();

    Matrix4 iview =
            Matrix4::Translation(position) *
            Matrix4::Rotation(yaw, Vector3(0, 1, 0)) *
            Matrix4::Rotation(pitch, Vector3(1, 0, 0));

    return iview;
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty
good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying
it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the
view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using
the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const Camera &c) {
    //Create our inverted matrix! Note how that to get a correct inverse matrix,
    //the order of matrices used to form it are inverted, too.
    Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());

    Vector2i screenSize = Window::GetWindow()->GetScreenSize();

    //Our mouse position x and y values are in 0 to screen dimensions range,
    //so we need to turn them into the -1 to 1 axis range of clip space.
    //We can do that by dividing the mouse values by the width and height of the
    //screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
    //and then subtracting 1 (-1.0 to 1.0).
    Vector4 clipSpace = Vector4(
            (position.x / (float)screenSize.x) * 2.0f - 1.0f,
            (position.y / (float)screenSize.y) * 2.0f - 1.0f,
            (position.z) - 1.0f,
            1.0f
    );

    //Then, we multiply our clipspace coordinate by our inverted matrix
    Vector4 transformed = invVP * clipSpace;

    //our transformed w coordinate is now the 'inverse' perspective divide, so
    //we can reconstruct the final world space by dividing x,y,and z by w.
    return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

