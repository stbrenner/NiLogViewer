NI Log Viewer
=============

Viewer for enteo NetInstall and FrontRange DSM log files.

Binary
------

**[NiLogVwr_1.1.6.zip](http://www.stephan-brenner.com/downloads/NiLogVwr/NiLogVwr_1.1.6.zip)** (6 KB)

Operating system:	Windows XP or newer  
[Version history](src/VersionHistory.txt)


Screenshot
----------

[![Screenshot](http://www.stephan-brenner.com/downloads/NiLogVwr/nilogvwr-small.gif)](http://www.stephan-brenner.com/downloads/NiLogVwr/nilogvwr-big.gif)

Features
--------
* Syntax highlighting
* Files and folders in the log file are displayed as hyperlinks. This allows for example that you jump from log file to log file by clicking the apropiate links.
* Sidebar which displays the whole content of a log folder
* Sorting log foder after name, modified time, size, number of errors, number of warnings or severity
* Filtering log folder (e.g. “sicom” show all files containing that string)
* Connecting to remote computer’s log folder
* Search over all files in the log folder
* Tabbed view
* Deletion of old log files
* Sending log files as eMail

How To Build
------------

Open ```src\LogViewer.sln``` in Visual Studio 2012 or [Visual Studio Express 2012 for Windows Desktop](http://www.microsoft.com/visualstudio/eng/products/visual-studio-express-for-windows-desktop) and build all.
