// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: https://www.gnu.org/licenses/gpl-3.0          *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef STRUCTURES_H_8210478915019450901745
#define STRUCTURES_H_8210478915019450901745

#include <vector>
#include <memory>
#include <chrono>
#include <zen/zstring.h>
#include "../afs/abstract.h"


namespace fff
{
using AFS = AbstractFileSystem;

enum class CompareVariant
{
    timeSize,
    content,
    size
};


enum class SymLinkHandling
{
    exclude,
    direct,
    follow
};


enum class SyncDirection : unsigned char //save space for use in FileSystemObject!
{
    none,
    left,
    right
};


enum CompareFileResult
{
    FILE_EQUAL,
    FILE_LEFT_SIDE_ONLY,
    FILE_RIGHT_SIDE_ONLY,
    FILE_LEFT_NEWER,  //CompareVariant::timeSize only!
    FILE_RIGHT_NEWER, //
    FILE_DIFFERENT_CONTENT, //CompareVariant::content, CompareVariant::size only!
    FILE_DIFFERENT_METADATA, //both sides equal, but different metadata only: short name case
    FILE_CONFLICT
};
//attention make sure these /|\  \|/ three enums match!!!
enum CompareDirResult
{
    DIR_EQUAL              = FILE_EQUAL,
    DIR_LEFT_SIDE_ONLY     = FILE_LEFT_SIDE_ONLY,
    DIR_RIGHT_SIDE_ONLY    = FILE_RIGHT_SIDE_ONLY,
    DIR_DIFFERENT_METADATA = FILE_DIFFERENT_METADATA, //both sides equal, but different metadata only: short name case
    DIR_CONFLICT           = FILE_CONFLICT
};

enum CompareSymlinkResult
{
    SYMLINK_EQUAL           = FILE_EQUAL,
    SYMLINK_LEFT_SIDE_ONLY  = FILE_LEFT_SIDE_ONLY,
    SYMLINK_RIGHT_SIDE_ONLY = FILE_RIGHT_SIDE_ONLY,
    SYMLINK_LEFT_NEWER      = FILE_LEFT_NEWER,
    SYMLINK_RIGHT_NEWER     = FILE_RIGHT_NEWER,
    SYMLINK_DIFFERENT_CONTENT  = FILE_DIFFERENT_CONTENT,
    SYMLINK_DIFFERENT_METADATA = FILE_DIFFERENT_METADATA, //both sides equal, but different metadata only: short name case
    SYMLINK_CONFLICT        = FILE_CONFLICT
};


std::wstring getSymbol(CompareFileResult cmpRes);


enum SyncOperation
{
    SO_CREATE_NEW_LEFT,
    SO_CREATE_NEW_RIGHT,
    SO_DELETE_LEFT,
    SO_DELETE_RIGHT,

    SO_MOVE_LEFT_FROM, //SO_DELETE_LEFT    - optimization!
    SO_MOVE_LEFT_TO, //SO_CREATE_NEW_LEFT

    SO_MOVE_RIGHT_FROM, //SO_DELETE_RIGHT    - optimization!
    SO_MOVE_RIGHT_TO, //SO_CREATE_NEW_RIGHT

    SO_OVERWRITE_LEFT,
    SO_OVERWRITE_RIGHT,
    SO_COPY_METADATA_TO_LEFT,  //objects are already equal: transfer metadata only - optimization
    SO_COPY_METADATA_TO_RIGHT, //

    SO_DO_NOTHING, //nothing will be synced: both sides differ
    SO_EQUAL,      //nothing will be synced: both sides are equal
    SO_UNRESOLVED_CONFLICT
};

std::wstring getSymbol(SyncOperation op); //method used for exporting .csv file only!


struct DirectionSet
{
    SyncDirection exLeftSideOnly  = SyncDirection::right;
    SyncDirection exRightSideOnly = SyncDirection::left;
    SyncDirection leftNewer       = SyncDirection::right; //CompareVariant::timeSize only!
    SyncDirection rightNewer      = SyncDirection::left;  //
    SyncDirection different       = SyncDirection::none; //CompareVariant::content, CompareVariant::size only!
    SyncDirection conflict        = SyncDirection::none;
};

DirectionSet getTwoWayUpdateSet();

inline
bool operator==(const DirectionSet& lhs, const DirectionSet& rhs)
{
    return lhs.exLeftSideOnly  == rhs.exLeftSideOnly  &&
           lhs.exRightSideOnly == rhs.exRightSideOnly &&
           lhs.leftNewer       == rhs.leftNewer       &&
           lhs.rightNewer      == rhs.rightNewer      &&
           lhs.different       == rhs.different       &&
           lhs.conflict        == rhs.conflict;
}

enum class SyncVariant
{
    twoWay, //use sync-database to determine directions
    mirror, //predefined
    update, //
    custom, //use custom directions
};
struct SyncDirectionConfig
{
    SyncVariant var = SyncVariant::twoWay;
    DirectionSet custom; //sync directions for SyncVariant::custom
    bool detectMovedFiles = false; //variant-dependent: e.g. always active for SyncVariant::twoWay! => use functions below for evaluation!
};

bool detectMovedFilesSelectable(const SyncDirectionConfig& cfg);
bool detectMovedFilesEnabled   (const SyncDirectionConfig& cfg);

DirectionSet extractDirections(const SyncDirectionConfig& cfg); //get sync directions: DON'T call for SyncVariant::twoWay!

std::wstring getVariantName(std::optional<CompareVariant> var);
std::wstring getVariantName(std::optional<SyncVariant> var);

std::wstring getVariantNameWithSymbol(SyncVariant var);

inline
bool operator==(const SyncDirectionConfig& lhs, const SyncDirectionConfig& rhs)
{
    return lhs.var == rhs.var &&
           (lhs.var != SyncVariant::custom || lhs.custom == rhs.custom) && //no need to consider custom directions if var != CUSTOM
           lhs.detectMovedFiles == rhs.detectMovedFiles; //useful to remember this setting even if the current sync variant does not need it
    //adapt effectivelyEqual() on changes, too!
}
inline bool operator!=(const SyncDirectionConfig& lhs, const SyncDirectionConfig& rhs) { return !(lhs == rhs); }

inline
bool effectivelyEqual(const SyncDirectionConfig& lhs, const SyncDirectionConfig& rhs)
{
    return (lhs.var == SyncVariant::twoWay) == (rhs.var == SyncVariant::twoWay) && //either both two-way or none
           (lhs.var == SyncVariant::twoWay || extractDirections(lhs) == extractDirections(rhs)) &&
           detectMovedFilesEnabled(lhs) == detectMovedFilesEnabled(rhs);
}


struct CompConfig
{
    CompareVariant compareVar = CompareVariant::timeSize;
    SymLinkHandling handleSymlinks = SymLinkHandling::exclude;
    std::vector<unsigned int> ignoreTimeShiftMinutes; //treat modification times with these offsets as equal
};

inline
bool operator==(const CompConfig& lhs, const CompConfig& rhs)
{
    return lhs.compareVar             == rhs.compareVar &&
           lhs.handleSymlinks         == rhs.handleSymlinks &&
           lhs.ignoreTimeShiftMinutes == rhs.ignoreTimeShiftMinutes;
}
inline bool operator!=(const CompConfig& lhs, const CompConfig& rhs) { return !(lhs == rhs); }

inline
bool effectivelyEqual(const CompConfig& lhs, const CompConfig& rhs) { return lhs == rhs; } //no change in behavior


enum class DeletionPolicy
{
    permanent,
    recycler,
    versioning
};

enum class VersioningStyle
{
    replace,
    timestampFolder,
    timestampFile,
};

struct SyncConfig
{
    //sync direction settings
    SyncDirectionConfig directionCfg;

    DeletionPolicy handleDeletion = DeletionPolicy::recycler; //use Recycle Bin, delete permanently or move to user-defined location

    //versioning options
    Zstring versioningFolderPhrase;
    VersioningStyle versioningStyle = VersioningStyle::replace;

    //limit number of versions per file: (if versioningStyle != replace)
    int versionMaxAgeDays = 0; //<= 0 := no limit
    int versionCountMin   = 0; //only used if versionMaxAgeDays > 0 => < versionCountMax (if versionCountMax > 0)
    int versionCountMax   = 0; //<= 0 := no limit
};


inline
bool operator==(const SyncConfig& lhs, const SyncConfig& rhs)
{
    return lhs.directionCfg           == rhs.directionCfg      &&
           lhs.handleDeletion         == rhs.handleDeletion    &&      //!= DeletionPolicy::versioning => still consider versioningFolderPhrase: e.g. user temporarily
           lhs.versioningFolderPhrase == rhs.versioningFolderPhrase && //switched to "permanent" deletion and accidentally saved cfg => versioning folder is easily restored
           lhs.versioningStyle        == rhs.versioningStyle   &&
           (lhs.versioningStyle == VersioningStyle::replace ||
            (
                lhs.versionMaxAgeDays == rhs.versionMaxAgeDays &&
                (lhs.versionMaxAgeDays <= 0 ||
                 lhs.versionCountMin  == rhs.versionCountMin)  &&
                lhs.versionCountMax   == rhs.versionCountMax
            ));
    //adapt effectivelyEqual() on changes, too!
}
inline bool operator!=(const SyncConfig& lhs, const SyncConfig& rhs) { return !(lhs == rhs); }


inline
bool effectivelyEqual(const SyncConfig& lhs, const SyncConfig& rhs)
{
    return effectivelyEqual(lhs.directionCfg, rhs.directionCfg) &&
           lhs.handleDeletion == rhs.handleDeletion &&
           (lhs.handleDeletion != DeletionPolicy::versioning || //only evaluate versioning folder if required!
            (
                lhs.versioningFolderPhrase == rhs.versioningFolderPhrase &&
                lhs.versioningStyle        == rhs.versioningStyle        &&
                (lhs.versioningStyle == VersioningStyle::replace ||
                 (
                     lhs.versionMaxAgeDays == rhs.versionMaxAgeDays &&
                     (lhs.versionMaxAgeDays <= 0 ||
                      lhs.versionCountMin  == rhs.versionCountMin)  &&
                     lhs.versionCountMax   == rhs.versionCountMax
                 ))
            ));
}


enum class UnitSize
{
    none,
    byte,
    kb,
    mb
};

enum class UnitTime
{
    none,
    today,
    thisMonth,
    thisYear,
    lastDays
};

struct FilterConfig
{
    FilterConfig() {}
    FilterConfig(const Zstring& include,
                 const Zstring& exclude,
                 size_t   timeSpanIn,
                 UnitTime unitTimeSpanIn,
                 size_t   sizeMinIn,
                 UnitSize unitSizeMinIn,
                 size_t   sizeMaxIn,
                 UnitSize unitSizeMaxIn) :
        includeFilter(include),
        excludeFilter(exclude),
        timeSpan     (timeSpanIn),
        unitTimeSpan (unitTimeSpanIn),
        sizeMin      (sizeMinIn),
        unitSizeMin  (unitSizeMinIn),
        sizeMax      (sizeMaxIn),
        unitSizeMax  (unitSizeMaxIn) {}

    /*
    Semantics of PathFilter:
    1. using it creates a NEW folder hierarchy! -> must be considered by <Two way> variant! (fortunately it turns out, doing nothing already has perfect semantics :)
    2. it applies equally to both sides => it always matches either both sides or none! => can be used while traversing a single folder!
    */
    Zstring includeFilter = Zstr("*");
    Zstring excludeFilter;

    /*
    Semantics of SoftFilter:
    1. It potentially may match only one side => it MUST NOT be applied while traversing a single folder to avoid mismatches
    2. => it is applied after traversing and just marks rows, (NO deletions after comparison are allowed)
    3. => equivalent to a user temporarily (de-)selecting rows -> not relevant for <Two way> variant! ;)
    */
    size_t timeSpan = 0;
    UnitTime unitTimeSpan = UnitTime::none;

    size_t sizeMin = 0;
    UnitSize unitSizeMin = UnitSize::none;

    size_t sizeMax = 0;
    UnitSize unitSizeMax = UnitSize::none;
};

inline
bool operator==(const FilterConfig& lhs, const FilterConfig& rhs)
{
    return lhs.includeFilter == rhs.includeFilter &&
           lhs.excludeFilter == rhs.excludeFilter &&
           lhs.timeSpan      == rhs.timeSpan      &&
           lhs.unitTimeSpan  == rhs.unitTimeSpan  &&
           lhs.sizeMin       == rhs.sizeMin       &&
           lhs.unitSizeMin   == rhs.unitSizeMin   &&
           lhs.sizeMax       == rhs.sizeMax       &&
           lhs.unitSizeMax   == rhs.unitSizeMax;
}
inline bool operator!=(const FilterConfig& lhs, const FilterConfig& rhs) { return !(lhs == rhs); }

void resolveUnits(size_t timeSpan, UnitTime unitTimeSpan,
                  size_t sizeMin,  UnitSize unitSizeMin,
                  size_t sizeMax,  UnitSize unitSizeMax,
                  time_t&   timeFrom,   //unit: UTC time, seconds
                  uint64_t& sizeMinBy,  //unit: bytes
                  uint64_t& sizeMaxBy); //unit: bytes


struct LocalPairConfig //enhanced folder pairs with (optional) alternate configuration
{
    LocalPairConfig() {}

    LocalPairConfig(const Zstring& phraseLeft,
                    const Zstring& phraseRight,
                    const std::optional<CompConfig>& cmpCfg,
                    const std::optional<SyncConfig>& syncCfg,
                    const FilterConfig& filter) :
        folderPathPhraseLeft (phraseLeft),
        folderPathPhraseRight(phraseRight),
        localCmpCfg(cmpCfg),
        localSyncCfg(syncCfg),
        localFilter(filter) {}

    Zstring folderPathPhraseLeft;  //unresolved directory names as entered by user!
    Zstring folderPathPhraseRight; //

    std::optional<CompConfig> localCmpCfg;
    std::optional<SyncConfig> localSyncCfg;
    FilterConfig              localFilter;
};


inline
bool operator==(const LocalPairConfig& lhs, const LocalPairConfig& rhs)
{
    return lhs.folderPathPhraseLeft  == rhs.folderPathPhraseLeft  &&
           lhs.folderPathPhraseRight == rhs.folderPathPhraseRight &&
           lhs.localCmpCfg           == rhs.localCmpCfg  &&
           lhs.localSyncCfg          == rhs.localSyncCfg &&
           lhs.localFilter           == rhs.localFilter;
}
inline bool operator!=(const LocalPairConfig& lhs, const LocalPairConfig& rhs) { return !(lhs == rhs); }


enum class ResultsNotification
{
    always,
    errorWarning,
    errorOnly,
};


enum class PostSyncCondition
{
    completion,
    errors,
    success
};


struct MainConfiguration
{
    CompConfig   cmpCfg;       //global compare settings:         may be overwritten by folder pair settings
    SyncConfig   syncCfg;      //global synchronisation settings: may be overwritten by folder pair settings
    FilterConfig globalFilter; //global filter settings:          combined with folder pair settings

    LocalPairConfig firstPair; //there needs to be at least one pair!
    std::vector<LocalPairConfig> additionalPairs;

    std::map<AfsDevice, size_t /*parallel operations*/> deviceParallelOps; //should only include devices with >= 2  parallel ops

    bool ignoreErrors = false; //true: errors will still be logged
    size_t automaticRetryCount = 0;
    std::chrono::seconds automaticRetryDelay{5};

    Zstring postSyncCommand; //user-defined command line
    PostSyncCondition postSyncCondition = PostSyncCondition::completion;

    Zstring altLogFolderPathPhrase; //fill to use different log file folder (other than the default %appdata%\FreeFileSync\Logs)

    std::string emailNotifyAddress; //optional
    ResultsNotification emailNotifyCondition = ResultsNotification::always;
};


size_t getDeviceParallelOps(const std::map<AfsDevice, size_t>& deviceParallelOps, const AfsDevice& afsDevice);
void   setDeviceParallelOps(      std::map<AfsDevice, size_t>& deviceParallelOps, const AfsDevice& afsDevice, size_t parallelOps);
size_t getDeviceParallelOps(const std::map<AfsDevice, size_t>& deviceParallelOps, const Zstring& folderPathPhrase);
void   setDeviceParallelOps(      std::map<AfsDevice, size_t>& deviceParallelOps, const Zstring& folderPathPhrase, size_t parallelOps);

inline
bool operator==(const MainConfiguration& lhs, const MainConfiguration& rhs)
{
    return lhs.cmpCfg               == rhs.cmpCfg               &&
           lhs.syncCfg              == rhs.syncCfg              &&
           lhs.globalFilter         == rhs.globalFilter         &&
           lhs.firstPair            == rhs.firstPair            &&
           lhs.additionalPairs      == rhs.additionalPairs      &&
           lhs.deviceParallelOps    == rhs.deviceParallelOps    &&
           lhs.ignoreErrors         == rhs.ignoreErrors         &&
           lhs.automaticRetryCount  == rhs.automaticRetryCount  &&
           lhs.automaticRetryDelay  == rhs.automaticRetryDelay  &&
           lhs.postSyncCommand      == rhs.postSyncCommand      &&
           lhs.postSyncCondition    == rhs.postSyncCondition    &&
           lhs.altLogFolderPathPhrase == rhs.altLogFolderPathPhrase &&
           lhs.emailNotifyAddress   == rhs.emailNotifyAddress   &&
           lhs.emailNotifyCondition == rhs.emailNotifyCondition;
}


std::optional<CompareVariant>           getCompVariant(const MainConfiguration& mainCfg);
std::optional<SyncVariant> getSyncVariant(const MainConfiguration& mainCfg);


struct WarningDialogs
{
    bool warnFolderNotExisting          = true;
    bool warnFoldersDifferInCase        = true;
    bool warnDependentFolderPair        = true;
    bool warnDependentBaseFolders       = true;
    bool warnSignificantDifference      = true;
    bool warnNotEnoughDiskSpace         = true;
    bool warnUnresolvedConflicts        = true;
    bool warnModificationTimeError      = true;
    bool warnRecyclerMissing            = true;
    bool warnInputFieldEmpty            = true;
    bool warnDirectoryLockFailed        = true;
    bool warnVersioningFolderPartOfSync = true;
};
inline bool operator==(const WarningDialogs& lhs, const WarningDialogs& rhs)
{
    return lhs.warnFolderNotExisting          == rhs.warnFolderNotExisting     &&
           lhs.warnFoldersDifferInCase        == rhs.warnFoldersDifferInCase   &&
           lhs.warnDependentFolderPair        == rhs.warnDependentFolderPair   &&
           lhs.warnDependentBaseFolders       == rhs.warnDependentBaseFolders  &&
           lhs.warnSignificantDifference      == rhs.warnSignificantDifference &&
           lhs.warnNotEnoughDiskSpace         == rhs.warnNotEnoughDiskSpace    &&
           lhs.warnUnresolvedConflicts        == rhs.warnUnresolvedConflicts   &&
           lhs.warnModificationTimeError      == rhs.warnModificationTimeError &&
           lhs.warnRecyclerMissing            == rhs.warnRecyclerMissing       &&
           lhs.warnInputFieldEmpty            == rhs.warnInputFieldEmpty       &&
           lhs.warnDirectoryLockFailed        == rhs.warnDirectoryLockFailed   &&
           lhs.warnVersioningFolderPartOfSync == rhs.warnVersioningFolderPartOfSync;
}
inline bool operator!=(const WarningDialogs& lhs, const WarningDialogs& rhs) { return !(lhs == rhs); }
}

#endif //STRUCTURES_H_8210478915019450901745
