Feature: Project task association
  As someone collecting tasks
  I can associate a task or a note to a project
  In order to organize my work

  Scenario: Dropping a task on a project
    Given I display the "Inbox" page
    And there is an item named ""The Pragmatic Programmer" by Hunt and Thomas" in the central list
    When I drop the item on "Projects / Read List" in the page list
    And I display the "Projects / Read List" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | "Clean Code" by Robert C Martin               |
       | "Domain Driven Design" by Eric Evans          |
       | "The Pragmatic Programmer" by Hunt and Thomas |

  Scenario: Dropping a note on a project
    Given I display the "Inbox" page
    And there is an item named "21/04/2014 14:49" in the central list
    When I drop the item on "Projects / Backlog" in the page list
    And I display the "Projects / Backlog" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display          |
       | 21/04/2014 14:49 |

  Scenario: Dropping a task on a project from context central list
    Given I display the "Contexts / Chores" page
    And there is an item named "Buy kiwis" in the central list
    When I drop the item on "Projects / Birthday" in the page list
    And I display the "Projects / Birthday" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Buy kiwis                                     |
