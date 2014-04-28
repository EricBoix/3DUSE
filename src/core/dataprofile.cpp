////////////////////////////////////////////////////////////////////////////////
#include "dataprofile.hpp"
////////////////////////////////////////////////////////////////////////////////
namespace vcity
{
////////////////////////////////////////////////////////////////////////////////
DataProfile::DataProfile()
    : m_bboxLowerBound(), m_bboxUpperBound(), m_offset(), m_xStep(), m_yStep()
{

}
////////////////////////////////////////////////////////////////////////////////
DataProfile createDataProfileDefault()
{
    DataProfile dp;
    dp.m_offset.x = 643000.0;
    dp.m_offset.y = 6857000.0;

    return dp;
}
////////////////////////////////////////////////////////////////////////////////
DataProfile createDataProfileParis()
{
    DataProfile dp;
    dp.m_offset.x = 643000.0;
    dp.m_offset.y = 6857000.0;

    return dp;
}
////////////////////////////////////////////////////////////////////////////////
DataProfile createDataProfileLyon()
{
    DataProfile dp;
    dp.m_offset.x = 643000.0;
    dp.m_offset.y = 6857000.0;

    return dp;
}
////////////////////////////////////////////////////////////////////////////////
} // namespace vcity
////////////////////////////////////////////////////////////////////////////////
