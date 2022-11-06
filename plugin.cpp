#include "TranslateEffect.hpp"
#include "ArrayEffect.hpp"
#include "BoxUvEffect.hpp"
#include "CopyEffect.hpp"
#include "BoxGeneratorEffect.hpp"
#include "ExplodeEffect.hpp"
#include "PointAttributeMath.hpp"

#include <PluginSupport/MfxRegister>

MfxRegister(
    TranslateEffect,
    ArrayEffect,
    BoxUvEffect,
    CopyEffect,
    BoxGeneratorEffect,
    ExplodeEffect,
    PointAttributeMath
)

