
#ifndef H_BACT_BACTER_H
#define H_BACT_BACTER_H

#include "GameObject.h"

#include "../math/Math.h"
#include "../math/Vector2.h"

namespace rob
{
    class Random;
} // rob

namespace bact
{

    class ObjectArray;

    using namespace rob;

    const float BacterMinSize = 0.3f;

    class Bacter : public GameObject
    {
    public:
        static const int TYPE = 2;
    public:
        Bacter();

        void Setup(const GameObject *target, Random &random);

        void SetTarget(const GameObject *target)
        { m_target = target; }

//        int TakeHit(BacterArray &bacterArray, Random &random)
        int TakeHit(ObjectArray &bacterArray, Random &random);

        void ModifySize(float sizeMod);

        void Update(const GameTime &gameTime) override;

        void SplitSelf();

//        static void TrySplit(Bacter *bacter, BacterArray &bacterArray, Random &random)
        static void TrySplit(Bacter *bacter, ObjectArray &bacterArray, Random &random);

//        static bool Split(Bacter *bacter, BacterArray &bacterArray, Random &random)
        static bool Split(Bacter *bacter, ObjectArray &bacterArray, Random &random);

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms);

    private:
        float m_anim;
        float m_size, m_sizeMod;
        const GameObject *m_target;
        int m_points;
        float m_splitTimer;
        float m_readyToSplitTimer;
    };

} // bact

#endif // H_BACT_BACTER_H
