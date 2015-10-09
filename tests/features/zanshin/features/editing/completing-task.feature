Feature: Completing tasks
  As someone who just accomplished something
  I can mark a task as completed
  In order to have a warm feeling about getting it done

   Scenario: Checking task in the list
     Given I display the "Inbox" page
     And there is an item named "Buy cheese" in the central list
     When I check the item
     Then the task corresponding to the item is done

   Scenario: Checking task in the editor
     Given I display the "Inbox" page
     And there is an item named "Buy apples" in the central list
     When I open the item in the editor
     And I mark it done in the editor
     And I open the item in the editor again
     Then the task corresponding to the item is done
     And the editor shows the task as done

