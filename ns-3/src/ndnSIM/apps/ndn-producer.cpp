/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-producer.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>
#include <ndn-cxx/lp/tags.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.Producer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Producer);

TypeId
Producer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::Producer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<Producer>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&Producer::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&Producer::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&Producer::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&Producer::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&Producer::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&Producer::m_keyLocator), MakeNameChecker());
  return tid;
}

Producer::Producer()
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
Producer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
Producer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
Producer::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  Name dataName(interest->getName());

  // dataName.append(m_postfix);
  // dataName.appendVersion();

  increaseDataNum();
  //std::cout << "Data Num:" << getDataNum() << std::endl;


  auto data = make_shared<Data>();
  data->setName(dataName);
  if(getChoiceType() == 2){
	  Name funcName(interest->getFunctionFullName());
	  auto string = funcName.toUri();
	  //std::cout << "string:" << string << std::endl;

	  data->setTag<lp::FunctionNameTag>(make_shared<lp::FunctionNameTag>(funcName));
	  data->setTag<lp::PartialHopTag>(nullptr);
	  data->setTag<lp::CountTag>(nullptr);
	  data->setTag<lp::PreviousFunctionTag>(make_shared<lp::PreviousFunctionTag>(Name("")));
	  shared_ptr<lp::PartialHopTag> partialHopTag = data->getTag<lp::PartialHopTag>();
  }else if(getChoiceType() == 4){

    Name funcName(interest->getFunctionFullName());
	  auto string = funcName.toUri();
	  //std::cout << "string:" << string << std::endl;

    data->setFunction(funcName);

	  data->setTag<lp::FunctionNameTag>(nullptr);
	  data->setTag<lp::PartialHopTag>(nullptr);
	  data->setTag<lp::CountTag>(nullptr);
	  /*
	  data->setTag<lp::PartialHopTag>(make_shared<lp::PartialHopTag>(0));
	  data->setTag<lp::CountTag>(make_shared<lp::CountTag>(0));
	  shared_ptr<lp::PartialHopTag> partialHopTag = data->getTag<lp::PartialHopTag>();
	  */
	  //std::cout << "FuncNameTag: " << *(data->getTag<lp::FunctionNameTag>()) << ",the type name is "<< typeid(*(data->getTag<lp::FunctionNameTag>())).name() <<"PartialHopTag: "<< *(data->getTag<lp::PartialHopTag>()) << ", typeName is " << typeid(*(partialHopTag)).name() << ",CountTag: " << *(data->getTag<lp::CountTag>())<< std::endl;
  }
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
  //std::cout << "Interest servicetime: " << interest->getServiceTime().count() << std::endl;
  data->setServiceTime(interest->getServiceTime());
   //std::cout << "Producer servicetime: " << data->getServiceTime().count() << std::endl;

  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));


  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}

} // namespace ndn
} // namespace ns3
