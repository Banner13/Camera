
#include "com_camera.h"

using namespace VideoCamera;

bool ComCamera::SetProperty(Property property, const PropertyValue& value)
{
	switch (property) {
		case Property::PIXEL_FORMAT:
			break;
		case Property::RESOLUTION:
			break;
		case Property::FRAMERATE:
			break;
		case Property::EXPOSURE:
			break;
		case Property::GAIN:
			break;
		case Property::RW_PROPERTY:
		case Property::RO_PROPERTY:
		case Property::FORMAT_LIST:
			break;
	}

	return true;
}

PropertyValue ComCamera::GetProperty(Property property) const
{
	PropertyValue temp;

	switch (property) {
		case Property::PIXEL_FORMAT:
			break;
		case Property::RESOLUTION:
			break;
		case Property::FRAMERATE:
			break;
		case Property::EXPOSURE:
			break;
		case Property::GAIN:
			break;
		case Property::RW_PROPERTY:
		case Property::RO_PROPERTY:
		case Property::FORMAT_LIST:
			break;
	}


	return temp;
}

