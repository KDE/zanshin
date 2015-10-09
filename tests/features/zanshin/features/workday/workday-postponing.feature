Feature: Postponing a task
  As someone using tasks
  I can change the start or due date of a task
  In order to procrastinate

  Scenario: Setting a date's start date to a date in the future makes it disappear in the Workday page
    Given I display the "Workday" page
    And there is an item named "Errands" in the central list
    When I open the item in the editor
    And I change the editor start date to "2015-03-20"
    And I look at the central list
    And I list the items
    Then the list does not contain "Errands"

  Scenario: Setting a date's due date to a date in the future makes it disappear in the Workday page
    Given I display the "Workday" page
    And there is an item named "Buy kiwis" in the central list
    When I open the item in the editor
    And I change the editor due date to "2015-03-20"
    And I look at the central list
    And I list the items
    Then the list does not contain "Buy kiwis"
