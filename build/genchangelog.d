#!/usr/bin/env rdmd

// This script will generate a changelog by scanning the git commit history for special changelog lines.
// These lines follow the format:
//
// ~type: Change log entry
//
// Where type is one of the following:
// * Fix, Bug, Resolve (and any permutation of suffixes) – Denote Bug Fixes.
// * Change, Update, API (and any permutation of suffixes) – Denote IMPORTANT CHANGES.
// * Add, New, Feature (and any permutation of suffixes) – Denote New Features.
//
// Separate releases are partitioned according to their release tag (only tags following the format `release_x.y.z` are included).
//
// The script will also add additional entries found in the changelog_supp.txt file.
// Tag sections (format `tag: x.y.z` - where x.y.z denotes the version number of the release) denote which release the following entries are added to.
// If there is no matching tag/release for a release section, the script will include it in the closest subsequent match (with "Latest" being reserved
// for changes in master that are pending release).
//
// The script is expected to be run from the root of the repository and by default will gather all changes since release_0.9.0.
// You can adjust the to/from tags to search between via command line args, or the GitLab environment variables.

import std.stdio;
import std.string;
import std.regex : ctRegex, matchFirst;
import std.file : readText, exists;
import std.algorithm : splitter, filter, canFind, map;
import std.range : isInputRange;

// Describes a release section in the change log
struct ReleaseBlock
{
  string versionStr;  // Identifier for the release
  string dateStr;     // Date for the release
  string[] fixes;     // Bugfix entries
  string[] changes;   // Change entries
  string[] features;  // Feature entries

  this(string ver)
  {
    versionStr = ver;
  }
}

// Performs a version number comparison and returns the difference
int versionCompare(string ver1, string ver2, char separator = '.')
{
  import std.algorithm : cmp;
  import std.conv : to;
  return cmp!((a, b) => a.to!int < b.to!int)(ver1.split(separator), ver2.split(separator));
}

// Main Entry point
int main(string[] args)
{
  import std.process : executeShell, environment;
  import std.getopt;
  import std.range : zip;

  immutable char[] defaultTag = "HEAD";
  string fromTag = "release_0.9.0", toTag = null;

  // Command line switches
  auto helpInformation = getopt(
    args,
    "from|f", "The first tag or SHA-1 to search from. Default: " ~ fromTag, &fromTag,
    "to|t", "The last tag or SHA-1 to search to. Default: " ~ defaultTag, &toTag
  );

  if (helpInformation.helpWanted)
  {
    defaultGetoptPrinter("Usage instructions:", helpInformation.options);
    return 0;
  }

  // If we haven't explicitly specified the destination tag, attempt to use the gitlab variables
  if (toTag is null)
    toTag = environment.get("CI_BUILD_TAG");
  if (toTag is null)
    toTag = environment.get("CI_BUILD_REF");
  if (toTag is null)
    toTag = defaultTag;

  // Get the git ref log
  auto pid = executeShell("git log --decorate --date=short " ~ fromTag ~ "..." ~ toTag);
  if (pid.status != 0)
  {
    writefln("Error: %s", pid.output);
    return -1;
  }

  // Split the output into separate lines
  auto gitRefLines = pid.output.lineSplitter();

  // Bucket the commits (separated by release tags)
  auto re = ctRegex!(`^.*commit\s+\w+\s*\(.*tag:\s*release_(\S+),`);
  auto releaseBuckets = splitter!(a => a.matchFirst(re))(gitRefLines);

  // Make a list of the actual release numbers
  auto tags = filter!(a => a.matchFirst(re))(gitRefLines).map!(a => a.matchFirst(re)[1]);

  // Create the changelog
  remove("build/CHANGELOG");
  auto changeLogFile = File("build/CHANGELOG", "w");
  changeLogFile.writeln("EUCLIDEON PLATFORM CHANGELOG\n");

  // Parse master's commits
  ReleaseBlock[] releases;
  if (!releaseBuckets.front.empty)
    releases ~= createReleaseBlock("Latest", releaseBuckets.front);
  releaseBuckets.popFront();

  // Parse the commits for the remaining release buckets
  foreach (batch; zip(tags, releaseBuckets))
    releases ~= createReleaseBlock(batch[0], batch[1]);

  // Parse the changelog.data file for extra entries (if it exists)
  if (exists("build/changelog_supp.txt"))
  {
    auto dataFileLines = readText("build/changelog_supp.txt").lineSplitter();
    int releaseId = 0;

    foreach (line; dataFileLines)
    {
      // Update the release block with the line if it starts with the sentinal character
      if (line.startsWith('~'))
        updateReleaseBlock(releases[releaseId], line);

      // Lines that match "tag: x.y.z" denote release sections...
      else if (line.startsWith("tag:"))
      {
        // Scan thru the list of releases and find where the data file's section slots in
        // Future releases go into Master; sequential releases go into the following release
        releaseId = 0;
        for (int i = 1; i < releases.length; ++i)
        {
          if (versionCompare(releases[i].versionStr, line.chompPrefix("tag:").strip()) >= 0)
          {
            releaseId = i;
            break;
          }
        }
      }
    }
  }

  // Finally print out the releases to the logfile
  foreach (r; releases)
    printReleaseBlock(changeLogFile, r);

  return 0;
}

// Creates a new release block given the received data
ReleaseBlock createReleaseBlock(Range)(string release, Range commitDataLines) if(isInputRange!Range)
{
  auto rb = ReleaseBlock(release);

  // Trim any preceding lines until we hit the date line
  while (!startsWith(commitDataLines.front, "Date:"))
    commitDataLines.popFront();

  rb.dateStr = commitDataLines.front[$-10..$];
  commitDataLines.popFront();

  // Update the release block with info from the commit line
  foreach (line; commitDataLines)
    updateReleaseBlock(rb, line);

  return rb;
}

// Updates the specified release block with the received entry (if it is a valid tag)
void updateReleaseBlock(ref ReleaseBlock rb, string entry)
{
  // Check if the entry contains a special tag
  static auto re = ctRegex!(`^\s*~(\S+):\s*(\S+.*\S+)\s*$`);
  auto capture = matchFirst(entry, re);
  if (!capture.empty)
  {
    // Bug fix entries
    if (["fix", "bug", "resolve"].canFind!((string a, string b) => b.startsWith(a))(capture[1].toLower))
      rb.fixes ~= capture[2];
    // Changes entries
    else if (["change", "update", "api"].canFind!((string a, string b) => b.startsWith(a))(capture[1].toLower))
      rb.changes ~= capture[2];
    // Features entries
    else if (["add", "new", "feature"].canFind!((string a, string b) => b.toLower().startsWith(a))(capture[1].toLower))
      rb.features ~= capture[2];
  }
}

// Prints out the release block to the desired file
void printReleaseBlock(ref File outFile, ref const(ReleaseBlock) rb)
{
  // Prints out the specified section entry to the desired file
  void printSection(string section, const string[] entries)
  {
    outFile.writefln("%s:", section);
    foreach (entry; entries)
      outFile.writefln("* %s", entry);
    outFile.writeln();
  }
  
  // Print the release header for the changelog and include the release tag date
  outFile.writeln("--------------------------------------------");
  outFile.writefln("%s (%s)", rb.versionStr, rb.dateStr);
  outFile.writeln("--------------------------------------------\n");

  // Print out the various sections based on what we found
  if (rb.fixes.length || rb.changes.length || rb.features.length)
  {
    if (rb.changes.length)
      printSection("Important Changes", rb.changes);
    if (rb.features.length)
      printSection("New Features", rb.features);
    if (rb.fixes.length)
      printSection("Fixes", rb.fixes);
  }
  else
  {
    outFile.writeln("No Changes...\n");
  }
}
