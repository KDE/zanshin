Feature: Project rename
  As someone collecting tasks
  I can rename a project
  In order to refine my tasks organization

  Scenario: Renamed projects appear in the list
    Given I display the available pages
    When I rename a "project" named "Backlog" to "Party"
    And I list the items
    Then the list is:
       | display                           | icon              |
       | Inbox                             | mail-folder-inbox |
       | Workday                           | go-jump-today     |
       | Projects                          | folder            |
       | Projects / Party                  | view-pim-tasks    |
       | Projects / Prepare talk about TDD | view-pim-tasks    |
       | Projects / Read List              | view-pim-tasks    |
       | Contexts                          | folder            |
       | Contexts / Errands                | view-pim-notes    |
       | Contexts / Online                 | view-pim-notes    |
