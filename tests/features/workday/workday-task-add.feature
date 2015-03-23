Feature: Adding a task from the workday view
  As someone adding tasks
  I can input a task for today in the quick entry
  In order to have a new task that starts today

  Scenario: Tasks added from the workday view start today
    Given I display the "Workday" page
    When I look at the central list
    And I add a "task" named "Burn some confidential documents"
    And I list the items
    Then the list contains "Burn some confidential documents"
