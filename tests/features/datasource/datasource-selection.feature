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
       | 21/04/2014 14:49                              |
       | Buy apples                                    |
       | Buy pears                                     |
       | Errands                                       |

  Scenario: Checking impacts the inbox
    Given I display the "Inbox" page
    And there is an item named "TestData / Calendar1" in the available data sources
    When I check the item
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | 21/04/2014 14:49                              |
       | Buy cheese                                    |
       | Buy apples                                    |
       | Buy pears                                     |
       | Errands                                       |

  Scenario: Unchecking impacts project list
    Given there is an item named "TestData / Calendar1" in the available data sources
    When I uncheck the item
    And I display the available pages
    And I list the items
    Then the list is:
       | display                           |
       | Inbox                             |
       | Workday                           |
       | Projects                          |
       | Projects / Backlog                |
       | Contexts                          |
       | Contexts / Chores                 |
       | Contexts / Internet               |
       | Contexts / Online                 |
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
       | Inbox                             |
       | Workday                           |
       | Projects                          |
       | Projects / Backlog                |
       | Projects / Prepare talk about TDD |
       | Projects / Read List              |
       | Contexts                          |
       | Contexts / Chores                 |
       | Contexts / Internet               |
       | Contexts / Online                 |
       | Tags                              |
       | Tags / Philosophy                 |
       | Tags / Physics                    |
