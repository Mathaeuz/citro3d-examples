#include "exercises.h"
#include "ex/00.h"
#include "ex/01.h"
#include "ex/02.h"

const int getExerciseCount() { return 3; }

const char *getExerciseName(int id)
{
    switch (id)
    {
    default:
        return "TrisImmediate - Composite";
    case 1:
        return "Elements - Normal from position";
    case 2:
        return "Elements - Textured";
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
    case 2:
        run02();
        return;
    }
}