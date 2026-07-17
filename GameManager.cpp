#include "GameManager.h"

GameManager& GameManager::instance()
{
    static GameManager inst;
    return inst;
}
