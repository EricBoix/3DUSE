#ifndef REFERENCE_HPP
#define REFERENCE_HPP

#include "cityobject.hpp"
#include "documentObject.hpp"

namespace documentADE
{
  enum PurposeType {
      PT_EXISTING=0,
      PT_NEW_PROJECT
  };
  class CoveragePeriod {
  public:
      void setStartTime(QDateTime startTime);
      void setEndTime(QDateTime startTime);
      QDateTime getStartTime();
      QDateTime getEndTime();
  private:
      QDateTime startTime;
      QDateTime endTime;
  };
  class Reference: public citygml::Object
  {
  public:
      QDateTime getReferringDate();
      std::string getReferenceText();
      std::string getPurpose();
      PurposeType getPurposeType();
      CoveragePeriod getCoveragePeriod();
      DocumentObject* getReferenceDocumentObject();
      citygml::CityObject* getReferencedCityObject();

      void setReferringDate(QDateTime referringDate);
      void setReferenceText(std::string referenceText);
      void setPurpose(std::string purpose);
      void setPurposeType(PurposeType purpseType);
      void setCoveragePeriod(CoveragePeriod coveragePeriod);
      void setReference(citygml::CityObject*, DocumentObject*);
  private:
      citygml::CityObject *_cityObject;
      DocumentObject *_documentObject;
      QDateTime _referringDate;
      std::string _referenceText;
      std::string _purpose;
      PurposeType _purposeType;
      CoveragePeriod _coveragePeriod;

  };
}

#endif // REFERENCE_HPP

