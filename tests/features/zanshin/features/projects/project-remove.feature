Feature: Project destruction
  As someone collecting tasks
  I can delete a project
  In order to clean my tasks

  Scenario: Removing a simple project from the list
    Given I display the available pages
    When I remove a "project" named "Prepare talk about TDD"
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Workday                           | go-jump-today       |
       | Projects                          | folder              |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Party                  | view-pim-tasks      |
       | Projects / Read List              | view-pim-tasks      |
       | Contexts                          | folder              |
       | Contexts / Chores                 | view-pim-notes      |
       | Contexts / Internet               | view-pim-notes      |
       | Contexts / Online                 | view-pim-notes      |
