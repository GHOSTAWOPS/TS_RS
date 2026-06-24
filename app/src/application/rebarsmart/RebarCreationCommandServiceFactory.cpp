#include "application/rebarsmart/RebarCreationCommandServiceFactory.h"

namespace tsrs::application {

RebarCreationCommandService RebarCreationCommandServiceFactory::create(
    tsrs::domain::rebar::RebarModel* model)
{
    return RebarCreationCommandService(model, &fixDistanceGenerator());
}

const RebarSmartFixDistanceCenterlineGeneratorAdapter&
RebarCreationCommandServiceFactory::fixDistanceGenerator()
{
    static const RebarSmartFixDistanceCenterlineGeneratorAdapter generator;
    return generator;
}

} // namespace tsrs::application
