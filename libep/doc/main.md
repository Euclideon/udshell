Welcome to Euclideon Platform
=============================

The most exciting piece of software engineering since UD itself!

What is?
--------
Is a common, flexible and extensible foundation for future product developments.
It is not a raw SDK, but rather, a framework to support an ecosystem.

Why?
----
We have a somewhat fragmented suite of products. We want to be able to collect all of our products and manpower
into the same pool, allowing tighter interaction between products, and improving our small teams ability to
migrate between projects and collaborate/support eachother more effectively.

We also want to; maximise exposure and platform availability for our full suite of products, improve 'cross-talk'
or overlapping features between projects, offer an extensibility suite to customers who are not expert software
engineers.

It should be easy for customers to optimise our products for their particular applications or needs with minimal
intervention from us, as such, a key focus of Platform is the ability for users to script functionality in
established high-level languages.

Consider
--------
A scientist wants to render a dataset with some custom visualisation shaders for analysis.
They are probably a python programmer, and want to express some simple rules for colouring voxels.
They will access the model in the scene via the public API, and assign a simple function as a voxel shader.
The whole customisation is maybe 10 lines of code, and may be controlled by hotkey.

A web developer wants to add some markup to visualise points of interest in some real-estate scans being
presented on a webpage.
They are a JavaScript developer, and they want to add some hovering text into the scene.
They access the model metadata to get the bookmark locations via the public API, and add text rendering nodes to
the scene at the locations of the bookmarks, which will render with the scene.

A corporate Geoverse user wants to interact with their internal geospatial database.
They probably assign a C#/Java developer who will create a DataSource component which connects to their internal
database. Once they implement the proper interface, Geoverse can create an instance of the data source and refer
to their data transparently.

The nature of our industry is that we have no idea where our technology will end up, so we need to be ready to
support whatever scenario may arise.

Features
--------
  * Platform agnostic
  * Language agnostic
  * UI agnostic (kind of)
  * Very modular
  * Not a raw SDK; is an extensibility framework
  * Inclusive; scripters, non-programmers, should be able to add simple features/customisations

Structure
---------
Such OOP. Many abstract. Very class. Wow!

Structurally similar to what you would expect from a C#/Java application.

Features are built upon a set of extensible interfaces. 'Platform' provides a comprehensive abstraction/'meta'
system which makes functionality of all components available to any language or runtime.

Component
---------
'Component' is the atomic base class. It is responsible for expressing the runtime reflection, memory management,
and generic interaction with objects.

All components have a type descriptor, which allows runtime inspection of the type information, and public
interface.

Components express their public interface with a few simple concepts:
  * **Properties** - pseudo data members, expressed by a getter+setter pair
  * **Methods** - functions associated with a component instance
  * **Events** - like C#; users can subscribe to events with a callback function/delegate, which is called when the
    event is signalled
  * **Static functions** - can be called for a component type, no instance required

Class diagram
-------------
The public API contains base classes which represent points of extensibility. For example:
  * **Activity** - a top-level application/UI layout, ie; viewer, editor, analysis, etc
  * **Node** - an item in a scene graph (something to render, or store data spatially)
  * **Resource** - represents some piece of user data (images, data arrays, buffers, shaders, etc)
  * **DataSource** - load/save resources from some data source (files on disk, network, database, etc)
  * **Camera** - a base for custom cameras
  * **PluginLoader** - Loads plugins, may provide language bindings
  * Etc...

Activities
----------
'Activities' are the top level applications or UI layouts that present useful functionality to the user.
Expressing Activities as components allows us to create multiple instances of activities, or have many activities
active simultaneously, which may be useful for various workflows.

Activities effectively represent a single application, or a task that the user is performing. It is their job to
manage user input and UI layout + interaction.

Resources
---------
Kernel maintains a globally accessible resource pool, and all activities may draw from the shared pool. This
means, multiple activities can work together on a single piece of data; ie, one activity may edit, the other may
view, and the viewer is updating in realtime as the user edits.

UI
--
Kernel is agnostic to particular UI systems; that is dictated by the shell hosting the application, but UI is not
generic.
UI is expressed as a 'UIComponent' which should produce a UI layout for the host app's UI system, and express
interaction with the UI in the general terms defined by Component; properties, events, methods, etc, such that the rest
of the ecosystem has generic access to the UI. This way, the only part of the ecosystem that must be tailored for a
particular UI system are the UI components themselves.

Functional components should attempt to communicate with UI in abstract terms, making it easy to change the
UI system at some later time if desired. For instance, an existing Qt app might want to be re-skinned with a
HTML5 UI for the web; the only thing that should be required is for some developer to reproduce the UI as HTML5,
and expose the same properties, events, etc, from the HTML5 javascript code. The rest of the code should connect
up automatically, with no awareness the UI system has changed.

Shell
-----
'Shell' is a top-level application which doesn't do much except host a kernel instance, which can load a project
file, and provide a container for activities.
'Shell' is a Qt app, which requires that UI components it loads are QML.

Any number of alternative shell apps are possible which may have varying features or varying degrees of
flexibility. For instance, a web shell which hosts a kernel instance and uses HTML5 as UI and JS for script,
or a headless shell, perhaps a daemon which services network requests or RPC, or perform data processing
operations via the command line via scripted commands.

Plugins
-------
Platform has comprehensive support for plugins. Plugins add functionality to Platform by adding new Component types.

Plugin components may derive from base classes available in the public API, which each define their usage semantics.

Plugins may be authored in the users preferred language.

Language binding
----------------
While the native API can be accessed with strong typing, the public API supports typeless interaction, which
simplifies binding to languages like JavaScript, Lua, Python, shells.

This is achieved with a custom 'Variant' type (and lots of C++11 magic), which handles these interactions.

The requirement for C++ developers is that public API function arguments must be convertible to/from Variant.
Tools for conversion are provided, and simple to implement.

Macros + scripting
------------------
Kernel has the Lua VM built-in, which may be used as a general scripting language, or for recording and executing
macros.

A design goal for the project is to actively record user interaction to assist in creation of macros.
Plan: as the user interacts, their interactions will be echo-ed to a 'listener' log as lines of script commands
that would perform the action they just performed. They will then be able to copy and paste that code into a
script to save and bind to a button or hotkey, or tweak their macro a bit, using the auto-generated code as an
example.

Memory management
-----------------
Most allocations in kernel are automatically managed. Platform uses ref-counting internally for 'Component's,
shared strings, and objects like delegates, maps.
This marries well with other languages and VM's with alternative memory management strategies; we currently
interact with 2 languages that use garbage collection for instance, with no manual burden on the user.

Manual memory management is not an option; it is not reasonable to assert manual memory management responsibility
onto users of languages that don't have this requirement. They will just make mistakes.

Users should be free to use whatever memory management strategy they prefer, or is already in use within their
own ecosystem. As long as they respect Platform's object model, they shouldn't run into problems.

C++11
-----
Kernel makes extensive use of C++11, and the C++ public API also requires this. Users not able to use C++11 will
have to fall back to the C API (which could theoretically be wrapped for C++03 if desired).
