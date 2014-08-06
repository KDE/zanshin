Feature: Project creation
  As someone collecting tasks and notes
  I can create a project
  In order to organize my tasks and notes

  Scenario: New projects appear in the list
    Given I display the available pages
    When I add a project named "Birthday" in the source named "TestData/Calendar1"
    And I list the items
    Then the list is:
       | display              | icon                |
       | Inbox                | mail-folder-inbox   |
       | Projects             | folder              |
       | Projects / Read List | view-pim-tasks      |
       | Projects / Backlog   | view-pim-tasks      |
       | Projects / Birthday  | view-pim-tasks      |
