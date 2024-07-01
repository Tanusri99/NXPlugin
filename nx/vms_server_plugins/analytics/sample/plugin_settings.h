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
	const std::string kPersonObjectType = "aol.color_detection.person";
    const std::string kCarObjectType = "aol.color_detection.car";
	const std::string kBicycleObjectType = "aol.color_detection.bicycle";
    const std::string kMotorbikeObjectType = "aol.color_detection.motorbike";
	const std::string kAeroplaneObjectType = "aol.color_detection.aeroplane";
	const std::string kBusObjectType = "aol.color_detection.bus";
	const std::string kTrainObjectType = "aol.color_detection.train";
	const std::string kTruckObjectType = "aol.color_detection.truck";
	const std::string kBoatObjectType = "aol.color_detection.boat";
	// animals
	const std::string kDogObjectType = "aol.color_detection.dog";
	const std::string kCatObjectType = "aol.color_detection.cat";
	const std::string kHorseObjectType = "aol.color_detection.horse";
	// personal
	const std::string kBackpackObjectType = "aol.color_detection.backpack";
	const std::string kHandbagObjectType = "aol.color_detection.handbag";
	const std::string kCellPhoneObjectType = "aol.color_detection.cell_phone";

    const std::string kNewTrackEventType = "aol.color_detection.newTrackEvent";
	const std::string kEndTrackEventType = "aol.color_detection.endTrackEvent";

	const std::string kObjectDetectedEventType = "aol.color_detection.objectDetectEvent";
	const std::string kObjectCountEventType = "aol.color_detection.objectCountEvent";

	const std::string kThrowPluginDiagnostics = "nx.aol_corsight_plugin.diagnotics";
    const std::string kDetailedLogs = "nx.aol_corsight_plugin.detailed.logs";

    const std::string kFramePeriodSetting = "nx.aol_corsight_plugin.period";
    const std::string kDurationSettings = "nx.aol_corsight_plugin.duration";

    const std::string kPersonOfInterestObject = "nx.aol_corsight_plugin.person_identified";
    const std::string kPersonUnknownObject = "nx.aol_corsight_plugin.person_unidentified";

    const std::string kPersonFaceMatchEvent = "nx.aol_corsight_plugin.matchevent";
    const std::string kPersonFaceNoMatchEvent = "nx.aol_corsight_plugin.nomatchevent";
    
    const std::string kCorsightAppIP = "nx.aol_corsight_plugin.corsightAppIP";
    const std::string kCorsightAppPort = "nx.aol_corsight_plugin.corsigntAppPort";
    
    const std::string kMinConfidence = "nx.aol_corsight_plugin.minconfidence";
    
    
    const std::string kKafkaPort = "nx.aol_corsight_plugin.kafkaport";
    const std::string kKafkaIP = "nx.aol_corsight_plugin.kafka_IP";
    
    // device agents

    const std::string kAgeEnabled = "nx.aol_corsight_plugin.enabler.age_enabled";
    const std::string kGenderEnabled = "nx.aol_corsight_plugin.enabler.gender_enabled";
    const std::string kMaskEnabled = "nx.aol_corsight_plugin.enabler.mask_enabled";
    const std::string kPOIMatchEnabled = "nx.aol_corsight_plugin.enabler.poi_enabled";

    const std::string kDisplayColor = "nx.aol_corsight_plugin.enabler.display_color";

    // enum types
    const std::string kAgeEnum = "nx.aol_corsight_plugin.enabler.age_enum";
    const std::string kGenderEnum = "nx.aol_corsight_plugin.enabler.gender_enum";
    const std::string kMaskEnum = "nx.aol_corsight_plugin.enabler.mask_enum";
	
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
	const std::string kPolygonEnabled = "aol.color_detection.polygonEnabled";
	const std::string kPolygonRegion = "aol.color_detection.polygonRegion";
	const std::string kPolygonInsideOnly = "aol.color_detection.insideOnly";

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