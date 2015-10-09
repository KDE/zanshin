Feature: Task creation from a tag
    As someone collecting tasks and notes
    I can add a task directly associated to a tag
    In order to give my task some semantic
@wip
  Scenario: Task added from a tag appear in its list
    Given I display the "Tags / Physics" page
    When I add a "task" named "Study fluid mechanics"
    And I look at the central list
    When I list the items
    Then the list is:
       | display                              |
       | Study fluid mechanics                |
