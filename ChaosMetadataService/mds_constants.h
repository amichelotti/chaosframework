//
//  mds_constants.h
//  CHAOSFramework
//
//  Created by Claudio Bisegni on 21/01/15.
//  Copyright (c) 2015 INFN. All rights reserved.
//

#ifndef CHAOSFramework_mds_constants_h
#define CHAOSFramework_mds_constants_h
namespace chaos {
    namespace metadata_service {
static const char* OPT_BATCH_SANDBOX_SIZE          ="batch-sandbox-size";

static const char* OPT_SYNCTIME_ERROR		      = "timesync-max-error";
       
static const char* OPT_CRON_JOB_CHECK              ="cron-job-check-repeat-time";
static const char* OPT_CRON_JOB_AGEING_MANAGEMENT  ="cron-job-ageing-management-repeat-time";
        
static const char* OPT_ARCHIVER_NUM                        ="archiver-instances";
static const char* OPT_ARCHIVER_THREAD                     ="archiver-thread";
static const char* OPT_ARCHIVER_QUEUE_PUSH_TIMEOUT         ="archiver-queue-push-timeout";
    }
}
#endif
