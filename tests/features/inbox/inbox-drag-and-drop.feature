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

  Scenario: Dropping two tasks on another one makes them children
    Given I display the "Inbox" page
    And the central list contains items named:
        | display    |
        | Buy apples |
        | Buy pears  |
    When I drop items on "Errands" in the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | Errands / Buy apples                          |
       | Errands / Buy pears                           |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | 21/04/2014 14:49                              |

  Scenario: Dropping two child tasks on the inbox makes them top-level
    Given I display the "Inbox" page
    And the central list contains items named:
        | display    |
        | Errands / Buy apples |
        | Errands / Buy pears  |
    When I drop items on "Inbox" in the page list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | Buy apples                                    |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |

  Scenario: Dropping a task on the inbox removes it from it's associated project
    Given I display the "Projects / Prepare talk about TDD" page
    And there is an item named "Create Sozi SVG" in the central list
    When I drop the item on "Inbox" in the page list
    And I display the "Inbox" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | Buy apples                                    |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |
       | Create Sozi SVG                               |

  @wip
  Scenario: Dropping a task on the inbox removes it from all it's contexts
    Given I display the "Contexts / Chores" page
    And there is an item named "Buy kiwis" in the central list
    When I drop the item on "Inbox" in the page list
    And I display the "Inbox" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Errands                                       |
       | Buy apples                                    |
       | "The Pragmatic Programmer" by Hunt and Thomas |
       | Buy cheese                                    |
       | Buy pears                                     |
       | 21/04/2014 14:49                              |
       | Create Sozi SVG                               |
       | Buy kiwis                                     |
