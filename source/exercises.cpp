#include "exercises.h"
#include "ex/00-shader-per-eye.cpp"

const int getExerciseCount() { return 2; }

const char *getExerciseName(int id)
{
    switch (id)
    {
    default:
        return "Shader per Eye";
    case 1:
        return "Also Shader per Eye";
    }
}

void runExercise(int id)
{
    switch (id)
    {
    default:
        exerciseShaderPerEye();
        return;
    }
}