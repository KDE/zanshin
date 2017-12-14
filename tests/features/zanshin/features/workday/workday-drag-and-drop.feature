Feature: Drag and drop a task in workday view
  As someone reviewing his tasks in the workday view
  I can drag task within the workday view
  In order to change parent/child relationships

  Scenario: Parenting a task in the Workday page
    Given I display the "Workday" page
    And I add a "task" named "parent"
    And I add a "task" named "child"
    And there is an item named "parent" in the central list
    And there is an item named "child" in the central list
    When I drop the item on "parent" in the central list
    And I look at the central list
    And I list the items
    Then the list contains items named:
       | display                                                 |
       | parent                                                  |
       | parent / child                                          |

  Scenario: Deparenting a task in the Workday page
    Given I display the "Workday" page
    And I add a "task" named "parent"
    And there is an item named "parent" in the central list
    And I add a child named "child" under the task named "parent"
    And there is an item named "parent / child" in the central list
    When I drop the item on the blank area of the central list
    And I look at the central list
    And I list the items
    Then the list contains items named:
       | display                                                 |
       | parent                                                  |
       | child                                                   |

