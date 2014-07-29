Feature: Completing tasks
  As someone who just accomplished something
  I can mark a task as completed
  In order to have a warm feeling about getting it done

   Scenario: Checking task in the list
   Given I'm looking at the inbox view
   And an item named "Buy cheese" in the central list
   When I check the item
   Then The task corresponding to the item is done

