Feature: Task creation from a context
    As someone collecting tasks
    I can add a task directly associated to a context
    In order to give my tasks some semantic
@wip
  Scenario: Task added from a context appear in its list
    Given I display the "Context / Online" page
    When I add a "task" named "Checking mail"
    And I look at the central list
    When I list the items
    Then the list is:
       | display                              |
       | Checking mail                        |
