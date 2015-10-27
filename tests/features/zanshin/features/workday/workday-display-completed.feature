Feature: Display completed tasks in Workday list
  As someone using tasks
  I can display the Workday list
  In order to know which tasks I’ve completed today (e.g. if done date is today)

  Scenario: The tasks that have been done today appear in the Workday list
    Given I display the "Inbox" page
    And there is an item named "Create Sozi SVG" in the central list
    When I check the item
    And I display the "Workday" page
    And I look at the central list
    And I list the items
    Then the list contains "Create Sozi SVG"
