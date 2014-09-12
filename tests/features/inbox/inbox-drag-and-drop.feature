Feature: Inbox task association
  As someone collecting tasks
  I can associate a task to another one
  In order to deal with complex tasks requiring several steps

  Scenario: Dropping a task on another one makes it a child
    Given I display the "Inbox" page
    And there is an item named "Buy apples" in the central list
    When I drop the item on "Errands" in the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | Errands / Buy apples                          |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |

  Scenario: Dropping a child task on the inbox makes it top-level
    Given I display the "Inbox" page
    And there is an item named "Errands / Buy apples" in the central list
    When I drop the item on "Inbox" in the page list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | Buy apples                                    |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |
