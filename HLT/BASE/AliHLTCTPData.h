//-*- Mode: C++ -*-
// $Id$

#ifndef ALIHLTCTPDATA_H
#define ALIHLTCTPDATA_H
//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//* See cxx source for full Copyright notice                               *

/** @file   AliHLTCTPData.h
    @author Matthias Richter
    @date   2009-08-20
    @brief  Container for CTP trigger classes and counters
*/

#include "TNamed.h"
#include "TClonesArray.h"
#include "TArrayL64.h"
#include "AliHLTLogging.h"
#include "AliHLTDataTypes.h"

/**
 * @class AliHLTCTPData
 * This is a container for the CTP trigger classes, the mapping to the bit
 * field, and counters.
 *
 * The object is also stored as part of the HLTGlobalTriggerDecision
 * @ingroup alihlt_trigger
 */
class AliHLTCTPData: public TNamed, public AliHLTLogging
{
 public:
  /// default constructor
  AliHLTCTPData();
  /// standard constructor including initialization from CTP_TRIGGER_CLASS
  AliHLTCTPData(const char* parameter);
  /// destructor
  virtual ~AliHLTCTPData();

  /**
   * Init the class ids and mapping from the CTP_TRIGGER_CLASS parameter.
   * The general format of the parameter is as follows:
   */
  int InitCTPTriggerClasses(const char* ctpString);

  /**
   * Evaluate an expression of trigger class ids with respect to the trigger mask.
   */
  bool EvaluateCTPTriggerClass(const char* expression, AliHLTComponentTriggerData& trigData) const;

  /**
   * Reset all counters
   */
  void ResetCounters();

  /**
   * Get index of a trigger class in the tigger pattern
   */
  int Index(const char* name) const;

  /**
   * Increment counter for CTP trigger classes
   * @param classIds  comma separated list of class ids
   */
  void Increment(const char* classIds);

  /**
   * Increment counter for CTP trigger classes
   * @param triggerPattern  corresponds to the 50bit trigger mask in the CDH
   */
  void Increment(AliHLTUInt64_t triggerPattern);

  /**
   * Increment counter for a CTP trigger class
   * @param classIdx  index of the class in the 50bit trigger mask
   */
  void Increment(int classIdx);

  /**
   * Increment counters according to the trigger data struct.
   * First extract trigger pattern from the CDH and then
   * increment from the trigger pattern.
   */
  int Increment(AliHLTComponentTriggerData& trigData);

  /**
   * Inherited from TObject, this prints the contents of the trigger decision.
   */
  virtual void Print(Option_t* option = "") const;

  AliHLTUInt64_t   Mask() const { return fMask; }
  const TArrayL64& Counters() const { return fCounters; }
  AliHLTUInt64_t   Counter(int index) const;
  AliHLTUInt64_t   Counter(const char* classId) const;
  const char*      Name(int index) const;

 protected:
 private:
  /// copy constructor prohibited
  AliHLTCTPData(const AliHLTCTPData&);
  ///assignment operator prohibited
  AliHLTCTPData& operator=(const AliHLTCTPData&);

  AliHLTUInt64_t fMask;      /// mask of initialized trigger classes
  TClonesArray   fClassIds;  /// array of trigger class ids
  TArrayL64      fCounters;  /// trigger class counters

  ClassDef(AliHLTCTPData, 0)
};

#endif
