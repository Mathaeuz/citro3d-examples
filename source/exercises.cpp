#include "exercises.h"
#include "ex/00.h"
#include "ex/01.h"

const int getExerciseCount() { return 2; }

const char *getExerciseName(int id)
{
    switch (id)
    {
    default:
        return "Tri - Shader per Eye";
    case 1:
        return "Prim - Render";
    }
}

void runExercise(int id)
{
    switch (id)
    {
    default:
        run00();
        return;
    case 1:
        run01();
        return;
    }
}