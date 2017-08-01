Feature: Project destruction
  As someone collecting tasks
  I can delete a project
  In order to clean my tasks

  Scenario: Removing a simple project from the list
    Given I display the available pages
    When I remove the page named "Prepare talk about TDD" under "Projects / Calendar1"
    And I list the items
    Then the list is:
       | display                          | icon                |
       | Inbox                            | mail-folder-inbox   |
       | Workday                          | go-jump-today       |
       | Projects                         | folder              |
       | Projects / Calendar1             | folder              |
       | Projects / Calendar1 / Read List | view-pim-tasks      |
       | Projects / Calendar2             | folder              |
       | Projects / Calendar2 / Backlog   | view-pim-tasks      |
       | Contexts                         | folder              |
       | Contexts / Errands               | view-pim-notes      |
       | Contexts / Online                | view-pim-notes      |
