Feature: Removing a task
  As someone using tasks
  I can remove a task
  In order to clean up

  Scenario: Removing a task that appear in the Workday list
    Given I display the "Workday" page
    And there is an item named "Buy pears" in the central list
    When I remove the item
    And I list the items
    Then the list does not contain "Buy pears"
