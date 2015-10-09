Feature: Modifying a task's due date
As someone adding a task
I can set the due date by editing the field "due date"
In order to have a task that ends at the entered date

  Scenario: Setting a date's due date to today makes it appear in the Workday page
    Given I display the "Inbox" page
    And I add a "task" named "Make more test tasks"
    And there is an item named "Make more test tasks" in the central list
    When I open the item in the editor
    And I change the editor due date to "2015-03-10"
    And I display the "Workday" page
    And I look at the central list
    And I list the items
    Then the list contains "Make more test tasks"

  Scenario: Setting a date's due date to a date in the past makes it appear in the Workday page
    Given I display the "Inbox" page
    And I add a "task" named "Buy potatoes"
    And there is an item named "Buy potatoes" in the central list
    When I open the item in the editor
    And I change the editor due date to "2001-03-10"
    And I display the "Workday" page
    And I look at the central list
    And I list the items
    Then the list contains "Buy potatoes"

