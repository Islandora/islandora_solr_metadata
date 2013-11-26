Islandora Solr Metadata Module
==============================
Current build status:
[![Build Status](https://travis-ci.org/discoverygarden/islandora_solr_metadata.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_solr_metadata)

CI Server:
http://jenkins.discoverygarden.ca


SUMMARY
-------
Provides an interface to construct configurations used for displaying metadata on Islandora objects.

REQUIREMENTS
------------
- [Islandora](http://github.com/Islandora/islandora)
- [Islandora Solr Search](http://github.com/Islandora/islandora_solr_search)

INSTALLATION
------------
Download and enable the module.

CONFIGURATION
-------------
The Islandora Solr Metadata module is used by selecting it to be the default metadata display viewer at
"your_site/admin/islandora/metadata". Once this is done you can begin to make configurations as you see fit.

CUSTOMIZATION
-------------
The backbone of this module is to allow users to select fields indexed in their Solr as what drives metadata displays. This allows for the creation of heterogenous displays pulled from many sources from something that is already easily available.

The Islandora Solr Metadata module uses templates to fuel the markup displayed when it's the defined viewer
for an object. As such these are overwritable to alter the display of the metadata as seen fit.

For a more indepth look at the metadata display framework and an example module implementation see the [Islandora wiki](http://github.com/Islandora/islandora/wiki/Metadata-Display-Viewers).

It's to be noted that you can have a content model associated with more than one configuration at a time. Similarily, on objects with two content models, two or more configurations could respond to display the markup for the object. These cases are handled by merging the displays based around the weight. Take for example the case where you have two responding configurations where the first configuration contains the a and c fields and the second the b and d fields. The metadata display output would then be in the following order: a, b, c, d. As such, it's at the discretion of the user, through the creation of configurations, to determine how they want their metadata to be displayed.

TROUBLESHOOTING
---------------

F.A.Q.
------

CONTACT
-------

SPONSORS
--------
