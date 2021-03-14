# Zanshin

A Getting Things Done application which aims at getting your mind like water.

> Zanshin is a state of awareness, of relaxed alertness, in Japanese martial arts. A literal translation of zanshin is "remaining mind". ([Extract from Wikipedia](https://en.wikipedia.org/wiki/Zanshin))

## Contributing

#### Documentation
- How to start contributing to KDE: https://community.kde.org/Get_Involved/development.
- Check out [HACKING](https://invent.kde.org/pim/zanshin/-/blob/master/HACKING).
- Talks that will help understand the code base and how to work with it:
	- [KDE Craftsmen](https://mirror.kumi.systems/kde/files/akademy/2014/videos/KDE_Craftsmen_-_Kevin_Ottens.webm)
	- [Agile to the Rescue](https://mirror.kumi.systems/kde/files/akademy/2014/videos/Agile_to_the_Rescue_-_Kevin_Ottens.webm)
		- There is an excelent free EPUB that introduces DDD (mentioned in the talk): [Domain Driven Design Quickly  by Abel Avram and Floyd Marinescu](https://www.infoq.com/minibooks/domain-driven-design-quickly/).
	- [Rebooting Zanshin](https://mirror.kumi.systems/kde/files/akademy/2014/videos/Rebooting_Zanshin_-_Kevin_Ottens.webm)
	- [Do As I Say, Not As I Do: Or is it the Other Way Around?](https://youtu.be/dgV_hGoOjiM)
    

#### Dependencies

- Make sure to have Qt > 5.12, KF5Akonadi, AkonadiCalendar, KontactInterface, Runner, WindowSystem, I18n.
	- For openSUSE Tumbleweed users:
		```sh
		$ zypper in akonadi-server-devel \
		akonadi-calendar-devel akonadi-contact-devel \
		kontactinterface-devel \
		krunner-devel \
		kwindowsystem-devel \
		ki18n-devel
		```

- Alternatively you can try [kdesrc-build](https://community.kde.org/Get_Involved/development#Set_up_kdesrc-build) or [KDE PIM Docker image for development](https://community.kde.org/KDE_PIM/Docker).

#### Issues and TO DO

- Check the [issue tracker](https://invent.kde.org/pim/zanshin/-/issues).
- Search in the [KDE Bugtracking System](https://bugs.kde.org/buglist.cgi?quicksearch=zanshin).

If you have any questions, see how to get in touch at [https://community.kde.org/KDE_PIM/Development](https://community.kde.org/KDE_PIM/Development#Additional_information).