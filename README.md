# Project Description: MMITSS 
 The Multi-Modal Intelligent Traffic Signal System (MMITSS) project is part of the Connected Vehicle Pooled Fund Study (CV PFS) entitled “Program to Support the Development and Deployment of Connected Vehicle System Applications.” The CV PFS was developed by a group of state and local transportation agencies and the Federal Highway Administration (FHWA). The Virginia Department of Transportation (VDOT) serves as the lead agency and is assisted by the University of Virginia’s Center for Transportation Studies, which serves as the technical and administrative lead for the PFS.

The USDOT identified six mobility application bundles under the Dynamic Mobility Applications (DMA) program for the connected vehicle environment where high-fidelity data from vehicles, infrastructure, pedestrians, etc. can be shared through wireless communications. Each bundle contains a set of related applications that are focused on similar outcomes. Since a major focus of the CV PFS members –who are the actual owners and operators of transportation infrastructure –lies in traffic signal related applications, the CV PFS team is leading the project entitled “Multi-Modal Intelligent Traffic Signal System” in cooperation with US DOT’s Dynamic Mobility Applications Program. As one of the six DMA application bundles, MMITSS includes five applications: Intelligent Traffic Signal Control (I-SIG), Transit Signal Priority (TSP), Mobile Accessible Pedestrian Signal System (PED-SIG), Freight Signal Priority (FSP), and Emergency Vehicle Preemption (PREEMPT).The MMITSS prototype was developed based on traffic controllers using the NTCIP communications protocol and new algorithms for providing priority control (emergency vehicle, transit, and truck priority) and intelligent signal control (e.g. adaptive control using connected vehicle data). 

The current implementation of MMITSS, based on the CV PFS Phase 3 – Deployment Readiness project, has been developed to be deployed on a variety of hardware platforms for both vehicles and at intersections. The Figure below shows the current MMITSS architecture. One of the objectives of the Phase 3 project was for the software to be hardware agnostic so that any vendor’s roadside units (RSU) and onboard units (OBU) could be interfaced to the processors.


![MMITSS Arch](docs/image/mmitssArchitecture2021.png)

The MMITSS Architecture includes OBUs and RSUs that provide wireless communications. The current communications is based on DSRC, but the FHWA RSU 4.1 Interface can support other communication technologies when they are available. Testing has occurred with RSUs and OBUs from Savari and Cohda however, the Savari OBU doesn’t support the RSU 4.1 Interface so these OBUs are only used for passenger vehicles that broadcast Basic Safety Messages (BSM). 

Associated with each RSU and OBU are the MMITSS Roadside Processor (MRP) and the MMITSS Vehicle-side Processor (VSP) where the core MMITSS software components are hosted. Both MRP and VSP have Message Transceivers that are responsible for encoding and decoding J2735 2016 messages into and from defined JSON messages that are used internally to MMITSS. If the standard changes or the encoding/decoding changes from UPER only the Message Transceivers have to be updated. Similarly, if the NTCIP standard for the traffic signal controller changes, such as the new NTCIP 1202 version 3.0, the changes only have to be made in the SNMP Engine. The components use common libraries that are available in the mmitss-common GitHub repository (https://github.com/mmitss/mmitss-common) including the MAP Engine and message library that encodes and decodes BSM, MAP, SPaT, SRM, and SSM messages. 

The MRP components include:
* *Priority Request Server* and *Solver* that implement the MMITSS Priority Control logic and determines a traffic signal schedule that accommodates a set of active priority requests (Zamanipour, M., Head, K.L., Feng, Y. and Khoshmagham, S. Efficient Priority Control Model for Multimodal Traffic Signals. Transportation Research Record: Journal of the Transportation Research Board, 2016. 2557(1): 86-99)
* *Coordination Request Generator* generates coordination priority requests based on a time of day pattern that provides cycle length, coordinated phase(s), offset, and phase splits
* *Trajectory Aware* that processes data from the BSM for performance measures and the adaptive traffic control component (I-SIG)
* *Traffic Control* (I-SIG) adaptive traffic signal control (not included in this release)
* *Traffic Control Interface* (TCI) that is responsible for taking the priority schedule and applying commands to make the traffic signal controller execute the desired schedule (uses NTCIP HOLD, FORCE-OFF, CALL, and OMIT commands)
* *SNMPEngine* which translates the TCI commands into NTCIP objects to send to the traffic signal controller
* *Performance Data Collector* that captures and logs system performance data (e.g. message counts, priority events granted and rejected, etc.)
* *MMITSS Web Interface* that allows a user to check the status, review and clear logs, and start and stop MMITSS components (using supervisorctl) and edit the system configuration parameters (e.g. IP addresses of the host MRP, RSU, Traffic Signal Controller, ports for socket communications, and other key MMITSS parameters)

The VSP components include:
* *Priority Request Generator* that is responsible for locating the vehicle on a MAP, determining the desired time of service, and forming a Signal Request Message (SRM) that requests priority service
* *Performance Data Collector* that captures and logs system performance data (e.g. message counts, priority events granted and rejected, etc.)
* *MMITSS Web Interface* that allows a user to check the status, review and clear logs, and start and stop MMITSS components (using supervisorctl) and edit the system configuration parameters (e.g. IP addresses of the host VSP, OBU, ports for socket communications, and other key MMITSS parameters)
* External to the VSP is a *MMITSS HMI* display and *HMI Controller* that can be executed on a laptop to allow visualization of SPaT data, vehicle data, SSM data, received MAPs, and status of the vehicle being on a MAP and sending an SRM

# Prerequisites

Requires:
* Ubuntu 18.04
* Docker

Other system requirements are defined in /docs/mmitss-deployment/mmitss-build-docker-image.docx

# Usage

## Building

Instructions for building MMITSS are in the /docs/mmitss-deployment/mmitss-build-docker-image.docx file 

## Testing

Tests are provided for many of the MMITSS components, but they are not currently automated. Where specific tests are provided, an input generator and output receiver (if used) is provided. For example see /src/mrp/traffic-controller-interface/tests or /src/mrp/priority-request-solver/test. Additional test scripts and automated testing are planned in future sprints.

## Deploying and Executing

### MMITSS

Instructions for deploying MMITSS are in /docs/mmitss-deployment/mmitss-field-deployment.docx

### MMITSS Simulation Platform

Instructions for setting up MMITSS Simulation using VISSIM are in /docs/mmitss-deloyment/mmitss-simulation-deployment.docx


# Additional Notes

__Known Issues:__ See [Issue Tracker](https://github.com/mmitss/mmitss-az/issues).

# Version History and Retention
__Status__: This project is in the release phase.

__Release Frequency__: This project is updated approximately once every 2-3 weeks

__Release History__: See [Releases](https://github.com/mmitss/mmitss-az/releases)

__Retention__: This project will remain publicly accessible for a minimum of five years (until at least 06/15/2025).


# License

By contributing to the Multi-Modal Intelligent Traffic Signal Systems (MMITSS) open source project, you agree that your contributions will be licensed under its Apache License 2.0 license.

# Contributions

## Issue tracker

[MMITSS AZ GitHub Issue Tracker Page](https://github.com/mmitss/mmitss-az/issues)

Contributors will use Github's issue tracking system to record and manage issues that are reported by users of MMITSS AZ in the field. These may include performance requests, defects, and new feature requests. The follow operating procedure highlights how the MMITSS AZ development team will address and respond to reported issues.

## Pull requests

[MMITSS AZ GitHub Pull Request Page](https://github.com/mmitss/mmitss-az/pulls)

All pull requests will be reviewed by the MMITSS AZ team. During the review of your pull request the team member will either merge it, request changes to it, or close it with an explanation. For major changes the reviewer may require additional support from the team, which could cause some delay. We'll do our best to provide updates and feedback throughout the process. Feel free to open pull requests, and the MMITSS team will communicate through it with any comments.
**Before submitting a pull request**, please make sure the following is done:
	
	1.	Fork the repository and create your branch from the develop branch.
	2.	If you've added code that should be tested, add tests!
	3.	Ensure the tests pass. Our target is 90% coverage
	4.	Update the documentation.
		- User QA procedures are documented within the Github Wiki
		- Architecture and user guide documentation should be included in the word document under the `docs/` folder
		- Please contact the MMITSS with qny questions
	5.	Format your code as outlined in the style guide


## Contributor Covenant Code of Conduct
#### Our Pledge
In the interest of fostering an open and welcoming environment, we as contributors and maintainers pledge to making participation in our project and our community a harassment-free experience for everyone, regardless of age, body size, disability, ethnicity, gender identity and expression, level of experience, nationality, personal appearance, race, religion, or sexual identity and orientation.

#### Our Standards
Examples of behavior that contributes to creating a positive environment include:
	
	-	Using welcoming and inclusive language
	-	Being respectful of differing viewpoints and experiences
	-	Gracefully accepting constructive criticism
	-	Focusing on what is best for the community
	-	Showing empathy towards other community members

Examples of unacceptable behavior by participants include:

	-	The use of sexualized language or imagery and unwelcome sexual attention or advances
	-	Trolling, insulting/derogatory comments, and personal or political attacks
	-	Public or private harassment
	-	Publishing others' private information, such as a physical or electronic address, without explicit permission
	-	Other conduct which could reasonably be considered inappropriate in a professional setting

#### Our Responsibilities
Project maintainers are responsible for clarifying the standards of acceptable behavior and are expected to take appropriate and fair corrective action in response to any instances of unacceptable behavior.
Project maintainers have the right and responsibility to remove, edit, or reject comments, commits, code, wiki edits, issues, and other contributions that are not aligned to this Code of Conduct, or to ban temporarily or permanently any contributor for other behaviors that they deem inappropriate, threatening, offensive, or harmful.

#### Scope
This Code of Conduct applies both within project spaces and in public spaces when an individual is representing the project or its community. Examples of representing a project or community include using an official project e-mail address, posting via an official social media account, or acting as an appointed representative at an online or offline event. Representation of a project may be further defined and clarified by project maintainers.
Enforcement

Instances of abusive, harassing, or otherwise unacceptable behavior may be reported by contacting the project team. All complaints will be reviewed and investigated and will result in a response that is deemed necessary and appropriate to the circumstances. The project team is obligated to maintain confidentiality with regard to the reporter of an incident. Further details of specific enforcement policies may be posted separately.

Project maintainers who do not follow or enforce the Code of Conduct in good faith may face temporary or permanent repercussions as determined by other members of the project's leadership.

# Contact Information

Contact Name: Larry Head

Contact Information: klhead@arizona.edu, (520) 621-2264

# Acknowledgements

To track how this government-funded code is used, we request that if you decide to build additional software using this code please acknowledge its Digital Object Identifier in your software’s README/documentation.

Digital Object Identifier: https://doi.org/10.21949/1520606
