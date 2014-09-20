// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#ifndef __CITYGML_EXPORTER_HPP__
#define __CITYGML_EXPORTER_HPP__
////////////////////////////////////////////////////////////////////////////////
#include <QDateTime>
////////////////////////////////////////////////////////////////////////////////
namespace citygml
{
////////////////////////////////////////////////////////////////////////////////
/// \brief Base class for export
///
class Exporter
{
public:
    Exporter();
    virtual ~Exporter();

    /// Enable or disable temporal export
    void setTemporalExport(bool param);

    /// Set temporal export date
    void setDate(const QDateTime& date);

    /// Set offset
    void setOffset(double offsetX, double offsetY);

protected:
    bool m_temporalExport;  ///< enable temporal export
    QDateTime m_date;       ///< date for temporal export

    double m_offsetX;       ///< x offset
    double m_offsetY;       ///< y offset

private:
};
////////////////////////////////////////////////////////////////////////////////
} // namespace citygml
////////////////////////////////////////////////////////////////////////////////
#endif // __CITYGML_EXPORTER_HPP__
