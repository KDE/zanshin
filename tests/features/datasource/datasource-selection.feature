Feature: Data sources selection
  As an advanced user
  I can select or deselect sources
  In order to see more or less content

  Scenario: Unchecking impacts the inbox
    Given I display the "Inbox" page
    And there is an item named "TestData / Calendar1" in the available data sources
    When I uncheck the item
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | Buy apples                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |

  Scenario: Checking impacts the inbox
    Given I display the "Inbox" page
    And there is an item named "TestData / Calendar1" in the available data sources
    When I check the item
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | Buy apples                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |

  Scenario: Unchecking impacts project list
    Given there is an item named "TestData / Calendar1" in the available data sources
    When I uncheck the item
    And I display the available pages
    And I list the items
    Then the list is:
       | display                           |
       | Contexts                          |
       | Contexts / Chores                 |
       | Contexts / Internet               |
       | Contexts / Online                 |
       | Inbox                             |
       | Projects                          |
       | Projects / Backlog                |
       | Tags                              |
       | Tags / Philosophy                 |
       | Tags / Physics                    |

  Scenario: Checking impacts project list
    Given there is an item named "TestData / Calendar1" in the available data sources
    When I check the item
    And I display the available pages
    And I list the items
    Then the list is:
       | display                           |
       | Contexts                          |
       | Contexts / Chores                 |
       | Contexts / Internet               |
       | Contexts / Online                 |
       | Inbox                             |
       | Projects                          |
       | Projects / Read List              |
       | Projects / Backlog                |
       | Projects / Prepare talk about TDD |
       | Tags                              |
       | Tags / Philosophy                 |
       | Tags / Physics                    |
