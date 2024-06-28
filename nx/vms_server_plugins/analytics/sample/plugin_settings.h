#pragma once

#ifndef PLUGIN_SETTING_H
#define PLUGIN_SETTING_H

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {
	
	const std::string kPluginType{ "Object Detection" };

	const std::string kPluginStatusMessage{ "pluginStatusMessage" };

#ifdef _WIN32
	const std::string kYoloDataPath{ "C:\\aol\\yolodata" };
#else
	const std::string kYoloDataPath{ "/usr/local/share" };
#endif

	const std::string kThrowPluginDiagnosticEventsFromEngineSetting{
		"throwPluginDiagnosticEventsFromEngine" };

	const std::string kTurnOnAnalytics{"turnOnAnalytics" };
	

	const std::string kUserEmail{ "userEmail" };
	const std::string kUserPassword{ "userPassword" };

	const std::string kLicenseKey{ "userLicenseKey" };
	// only if checkbox ticked, once applied turn off again
	const std::string kValidateApplyLicense{ "validateApplyLicense"};

	// read only
	const std::string kExpiryDate( "licenseExpiry");
	const std::string kNumberOfObjects{ "numberObjectsEnabled" };
	const std::string kDaysRemaining( "daysRemaining");
	const std::string kChannelsEnabled( "channelsEnabled");

	const std::string kSerialNumber("serialNumber");
	const std::string kMachineActivated("machineActivated");

	const std::string kLicenseWarning( "licenseWarning");
	// options
	const std::string kwarningOptionKeys[] = {"0","1","2","3","4"};
	const std::string kwarningOptions[] = {
		"Valid",
		"Under a year",
		"Under a month",
		"Under a week",
		"Invalid"
	};

	const int LICENSE_VALID=0;
	const int LICENSE_ONE_YEAR=1;
	const int LICENSE_ONE_MONTH=2;
	const int LICENSE_ONE_WEEK=3;
	const int LICENSE_INVALID=4;

	const std::string kSettingsError {"userSettingsError"};

	const std::string kYoloFolder{ "yoloFolder" };
	const std::string kCfgFilePath{ "cfgFilePath" };
	const std::string kWeightsPath{ "weightsPath" };
	const std::string kNamesPath{ "namesPath" };
	const std::string kBlobSize{ "blobSize" };
	const std::string kDetectionFramePeriodSetting{ "framePeriod" };
	const std::string kConfidenceSetting{ "confidenceThreshold" };
	const std::string kNMSSetting{ "nmsThreshold" };
	const std::string kDurationSettings{ "durationSetting" };
	const std::string kUseCUDA{ "useCUDA" };

	const int NUM_OBJECTS=15;
	// objects
	const std::string kPersonObjectType = "aol.object_detection.person";
    const std::string kCarObjectType = "aol.object_detection.car";
	const std::string kBicycleObjectType = "aol.object_detection.bicycle";
    const std::string kMotorbikeObjectType = "aol.object_detection.motorbike";
	const std::string kAeroplaneObjectType = "aol.object_detection.aeroplane";
	const std::string kBusObjectType = "aol.object_detection.bus";
	const std::string kTrainObjectType = "aol.object_detection.train";
	const std::string kTruckObjectType = "aol.object_detection.truck";
	const std::string kBoatObjectType = "aol.object_detection.boat";
	// animals
	const std::string kDogObjectType = "aol.object_detection.dog";
	const std::string kCatObjectType = "aol.object_detection.cat";
	const std::string kHorseObjectType = "aol.object_detection.horse";
	// personal
	const std::string kBackpackObjectType = "aol.object_detection.backpack";
	const std::string kHandbagObjectType = "aol.object_detection.handbag";
	const std::string kCellPhoneObjectType = "aol.object_detection.cell_phone";

    const std::string kNewTrackEventType = "aol.object_detection.newTrackEvent";
	const std::string kEndTrackEventType = "aol.object_detection.endTrackEvent";

	const std::string kObjectDetectedEventType = "aol.object_detection.objectDetectEvent";
	const std::string kObjectCountEventType = "aol.object_detection.objectCountEvent";
	
	// for quick lookup
	const std::string PERSON="person";
	const std::string CAR="car";
	const std::string BICYCLE="bicycle";
	const std::string MOTORBIKE="motorbike";
	const std::string AEROPLANE="aeroplane";
	const std::string BUS="bus";
	const std::string TRAIN="train";
	const std::string TRUCK="truck";
	const std::string BOAT="boat";
	const std::string DOG="dog";
	const std::string CAT="cat";
	const std::string HORSE="horse";
	const std::string BACKPACK="backpack";
	const std::string HANDBAG="handbag";
	const std::string CELLPHONE="cell phone";// from coco.names

	// user selection
	
	const std::string kEnablePerson{"enablePerson"};
	const std::string kEnableCar{"enableCar"};
	const std::string kEnableBicycle{"enableBicycle"};
	const std::string kEnableMotorbike{"enableMotorbike"};
	const std::string kEnableAeroplane{"enableAeroplane"};
	const std::string kEnableBus{"enableBus"};
	const std::string kEnableTrain{"enableTrain"};
	const std::string kEnableTruck{"enableTruck"};
	const std::string kEnableBoat{"enableBoat"};
	const std::string kEnableDog{"enableDog"};
	const std::string kEnableCat{"enableCat"};
	const std::string kEnableHorse{"enableHorse"};
	const std::string kEnableBackpack{"enableBackpack"};
	const std::string kEnableHandbag{"enableHandbag"};
	const std::string kEnablePhone{"enablePhone"};

	// region of interest
	const std::string kPolygonEnabled = "aol.object_detection.polygonEnabled";
	const std::string kPolygonRegion = "aol.object_detection.polygonRegion";
	const std::string kPolygonInsideOnly = "aol.object_detection.insideOnly";

	int getObjectNamesList(std::vector<std::string> &objectNamesList);

	int getObjectIndex(std::string objectName);
	bool objectOfInterest(std::vector<std::string> enabledObjectList, std::string objectName);
	const std::string & getKeyByName(std::string objName);
	bool getObjectName(size_t index, std::string &objName);

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif