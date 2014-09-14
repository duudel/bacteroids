
#include "BacteroidsGame.h"
#include "../application/GameState.h"

namespace bact
{

    using namespace rob;

    class BacteroidsState : public GameState
    {
    public:
//        BacteroidsState(MasterCache &cache, Renderer *)

        void Update(const GameTime &gameTime) override
        {

        }

        void Render(const GameTime &gameTime) override
        {

        }
    };

    bool Bacteroids::Initialize()
    {
        ChangeState<BacteroidsState>();
        return true;
    }

} // bact
