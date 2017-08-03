Feature: Task creation from a project
As someone collecting tasks
I can add a task directly inside a project
In order to organize my tasks

  Scenario: Task added from a project appear in its list
    Given I display the "Projects / TestData » Calendar1 » Calendar2 / Backlog" page
    When I add a "task" named "Buy a cake"
    And I add a "task" named "Buy a present"
    And I look at the central list
    When I list the items
    Then the list is:
       | display              |
       | Buy a cake           |
       | Buy a present        |
