Feature: Drop a task on workday view
  As someone reviewing his tasks
  I can drag a task from the current view and drop it in the workday view
  In order to have the task start today

  Scenario: Dropping a task on Workday page
    Given I display the "Inbox" page
    And I add a "task" named "Buy Pineapples"
    And there is an item named "Buy Pineapples" in the central list
    When I drop the item on "Workday" in the page list
    And I display the "Workday" page
    And I look at the central list
    And I list the items
    Then the list contains "Buy Pineapples"

  Scenario: Dropping two tasks on Workday page
    Given I display the "Inbox" page
    And I add a "task" named "Don't eat the cake"
    And I add a "task" named "The cake is a lie"
    And the central list contains items named:
      | display            |
      | Don't eat the cake |
      | The cake is a lie  |
    When I drop items on "Workday" in the page list
    And I display the "Workday" page
    And I look at the central list
    And I list the items
    Then the list contains "Don't eat the cake"
    And the list contains "The cake is a lie"
