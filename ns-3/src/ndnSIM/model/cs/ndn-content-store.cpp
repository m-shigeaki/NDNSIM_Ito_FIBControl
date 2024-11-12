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

#include "ndn-content-store.hpp"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "../ndn-cxx/lp/tags.hpp"


NS_LOG_COMPONENT_DEFINE("ndn.cs.ContentStore");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ContentStore);

TypeId
ContentStore::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ContentStore")
      .SetGroupName("Ndn")
      .SetParent<Object>()

      .AddTraceSource("CacheHits", "Trace called every time there is a cache hit",
                      MakeTraceSourceAccessor(&ContentStore::m_cacheHitsTrace),
                      "ns3::ndn::ContentStore::CacheHitsCallback")

      .AddTraceSource("CacheMisses", "Trace called every time there is a cache miss",
                      MakeTraceSourceAccessor(&ContentStore::m_cacheMissesTrace),
                      "ns3::ndn::ContentStrore::CacheMissesCallback");

  return tid;
}

ContentStore::~ContentStore()
{
}

namespace cs {

//////////////////////////////////////////////////////////////////////

Entry::Entry(Ptr<ContentStore> cs, shared_ptr<const Data> data)
  : m_cs(cs)
  , m_data(data)
  , m_latency(0)
  , m_time(0)
{
}

Entry::Entry(Ptr<ContentStore> cs, shared_ptr<const Data> data, int latency, long long currenttime)
  : m_cs(cs)
  , m_data(data)
  , m_latency(latency)
  , m_time(currenttime)
{
}

const Name&
Entry::GetName() const
{
  return m_data->getName();
}

shared_ptr<const Data>
Entry::GetData() const
{
  return m_data;
}

Ptr<ContentStore>
Entry::GetContentStore()
{
  return m_cs;
}

/*
const Name&
Entry::GetFunction()
{
  Name funcname;
  bool hasFunction = false;
  if(m_data->getTag<lp::FunctionNameTag>() != nullptr){
    funcname = *(m_data->getTag<lp::FunctionNameTag>()); 
    hasFunction = true;
  }
  return funcname;
}
*/

///*
const bool
Entry::hasFunction()
{
  if(m_data->getTag<lp::FunctionNameTag>() != nullptr){
    Name funcname = *(m_data->getTag<lp::FunctionNameTag>()); 
    std::string funcstr = funcname.toUri();
    return true;
  }else{
    return false;
  }
}
//*/
const int
Entry::GetLatency()
{
  return m_latency;
}

const long long
Entry::GetCurrentTime()
{
  return m_time;
}

void
Entry::UpdateCurrentTime()
{
  auto now = time::steady_clock::now();
  auto nowtime = time::duration_cast<time::nanoseconds>(now.time_since_epoch());
  m_time = nowtime.count();
}

} // namespace cs
} // namespace ndn
} // namespace ns3
