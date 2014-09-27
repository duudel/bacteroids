
#ifndef H_BACT_BACTER_H
#define H_BACT_BACTER_H

#include "GameObject.h"

namespace rob
{
    class Random;
} // rob

namespace bact
{

    class ObjectArray;

    using namespace rob;

    class Bacter : public GameObject
    {
    public:
        static const int TYPE = 2;
    public:
        Bacter();

        void SetTarget(const GameObject *target)
        { m_target = target; }
        void ModifySize(float sizeMod)
        { m_sizeMod = Max(m_sizeMod, sizeMod); }

        void CopyAttributes(const Bacter *other)
        {
            m_position = other->m_position;
            m_velocity = other->m_velocity;
            m_radius = other->m_radius;
            m_alive = other->m_alive;
            m_anim = other->m_anim;
            m_size = other->m_size;
            m_sizeMod = other->m_sizeMod;
            m_target = other->m_target;
            m_points = other->m_points;
            m_splitTimer = other->m_splitTimer;
            m_readyToSplitTimer = other->m_readyToSplitTimer;
        }

        void RandomizeAnimation(Random &random);

        int GetPoints() const
        { return m_points; }

        bool WantsToSplit() const;
        bool CanSplit() const;
        bool DiesIfSplits() const;
        void Split(Random &random);

        void Update(const GameTime &gameTime) override;

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
