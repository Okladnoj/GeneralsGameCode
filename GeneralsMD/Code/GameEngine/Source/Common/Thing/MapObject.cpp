#include "Common/MapObject.h"
#include "Common/ThingTemplate.h"
#include "PreRTS.h"
#include <stdio.h>

MapObject *MapObject::TheMapObjectListPtr = nullptr;
Dict MapObject::TheWorldDict;

MapObject::MapObject(Coord3D loc, AsciiString name, Real angle, Int flags,
                     const Dict *props, const ThingTemplate *thingTemplate)
    : m_location(loc), m_objectName(name), m_thingTemplate(thingTemplate),
      m_angle(angle), m_nextMapObject(TheMapObjectListPtr), m_flags(flags),
      m_properties(props ? *props : Dict()), m_color(0), m_renderObj(nullptr),
      m_shadowObj(nullptr), m_runtimeFlags(0) {

  TheMapObjectListPtr = this;
  for (int i = 0; i < BRIDGE_MAX_TOWERS; ++i)
    m_bridgeTowers[i] = nullptr;
}

MapObject::~MapObject(void) {
  if (TheMapObjectListPtr == this) {
    TheMapObjectListPtr = m_nextMapObject;
  } else {
    MapObject *prev = TheMapObjectListPtr;
    while (prev && prev->m_nextMapObject != this) {
      prev = prev->m_nextMapObject;
    }
    if (prev) {
      prev->m_nextMapObject = m_nextMapObject;
    }
  }
  m_nextMapObject = nullptr;
}

const ThingTemplate *MapObject::getThingTemplate(void) const {
  return m_thingTemplate;
}

void MapObject::setThingTemplate(const ThingTemplate *thing) {
  m_thingTemplate = thing;
}

void MapObject::setName(AsciiString name) { m_objectName = name; }

MapObject *MapObject::duplicate(void) {
  return newInstance(MapObject)(m_location, m_objectName, m_angle, m_flags,
                                &m_properties, m_thingTemplate);
}

void MapObject::setRenderObj(RenderObjClass *pObj) { m_renderObj = pObj; }

RenderObjClass *MapObject::getBridgeRenderObject(BridgeTowerType type) {
  return m_bridgeTowers[type];
}

void MapObject::setBridgeRenderObject(BridgeTowerType type,
                                      RenderObjClass *renderObj) {
  m_bridgeTowers[type] = renderObj;
}

WaypointID MapObject::getWaypointID() {
  return (WaypointID)0; // STUB
}

AsciiString MapObject::getWaypointName() {
  return ""; // STUB
}

void MapObject::setWaypointID(Int i) {}

void MapObject::setWaypointName(AsciiString n) {}

void MapObject::validate(void) {}

void MapObject::verifyValidTeam(void) {}

void MapObject::verifyValidUniqueID(void) {}

void MapObject::fastAssignAllUniqueIDs(void) {}

Int MapObject::countMapObjectsWithOwner(const AsciiString &n) { return 0; }
